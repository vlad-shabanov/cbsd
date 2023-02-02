// CBSD Project 2017-2018
// CBSD Team <cbsd+subscribe@lists.tilda.center>
// 0.3
// this is very experimental and quickly written code. a lot of refactoring is needed.
// TODO: prometheus accept/bind socket thread: drop privileges to nobody users instead of root
#include <sys/param.h>
#include <sys/jail.h>

#include <jail.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/rctl.h>
#include <sys/sysctl.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <libutil.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>

#include <sys/user.h>
#include <sys/stat.h>

#include <libprocstat.h>
#include <limits.h>
#include <dirent.h>

#include <paths.h>

#include <assert.h>
#include <inttypes.h>

#include "beanstalk.h"

//prom
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//

#include <pthread.h>
#include "sqlite3.h"

#include "racct-generic-stats.c"

#define JP_USER 0x01000000
#define JP_OPT 0x02000000

#define PRINT_DEFAULT 0x01
#define PRINT_HEADER 0x02
#define PRINT_NAMEVAL 0x04
#define PRINT_QUOTED 0x08
#define PRINT_SKIP 0x10
#define PRINT_VERBOSE 0x20
#define PRINT_JAIL_NAME 0x40

static struct jailparam *params;
static int *param_parent;
static int nparams;
unsigned int running_jails;

int accept_busy=0;
// not for global:
struct sockaddr_in cli_addr;
int listenfd = 0;

//prom
/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

int list_data();

int
sum_data()
{
	struct item_data *target = NULL;
	struct item_data *ch;
	struct item_data *next_ch;
	char sql[512];
	char stats_file[1024];
	const char *hostname = getenv(
	    "HOST"); // Still banging the env every second or so, only do this
		     // at load?
	int ret = 0;
	FILE *fp;
	char json_str[20000]; // todo: dynamic from number of bhyve/jails
	char json_buf[1024];  // todo: dynamic from number of bhyve/jails
	int i;
	struct timeval now_time;
	int cur_time = 0;
	int round_total = save_loop_count + 1;

	struct sum_item_data *newd;
	struct sum_item_data *temp;
	struct sum_item_data *sumch;
	struct sum_item_data *next_sumch;

	tolog(log_level, "\n ***---calc jail avgdata---*** \n");

	gettimeofday(&now_time, NULL);
	cur_time = (time_t)now_time.tv_sec;

	for (ch = item_list; ch; ch = ch->next) {
		if (strlen(ch->orig_name) < 1) {
			continue;
		}
		if (ch->modified == 0) {
			continue;
		}
		i = sum_jname_exist(ch->orig_name);

		if (i) {
			for (sumch = sum_item_list; sumch;
			     sumch = sumch->next) {
				if (!strcmp(ch->orig_name, sumch->name)) {
					sumch->modified += ch->modified;
					sumch->pcpu += ch->pcpu;
					sumch->memoryuse += ch->memoryuse;
					sumch->maxproc += ch->maxproc;
					sumch->openfiles += ch->openfiles;
					sumch->readbps += ch->readbps;
					sumch->writebps += ch->writebps;
					sumch->readiops += ch->readiops;
					sumch->writeiops += ch->writeiops;
					sumch->pmem += ch->pmem;
					break;
				}
			}
		} else {
			CREATE(newd, struct sum_item_data, 1);
			newd->modified = ch->modified;
			newd->pcpu = ch->pcpu;
			newd->memoryuse = ch->memoryuse;
			newd->maxproc = ch->maxproc;
			newd->openfiles = ch->openfiles;
			newd->readbps = ch->readbps;
			newd->writebps = ch->writebps;
			newd->readiops = ch->readiops;
			newd->writeiops = ch->writeiops;
			newd->pmem = ch->pmem;
			newd->next = sum_item_list;
			sum_item_list = newd;
			strcpy(newd->name, ch->orig_name);
			tolog(log_level,
			    "[AVGSUM] !! %s struct has been added\n",
			    newd->name);
		}
	}

	memset(json_str, 0, sizeof(json_str));
	for (sumch = sum_item_list; sumch; sumch = sumch->next) {
		if (strlen(sumch->name) < 1) {
			continue;
		}
		tolog(log_level,
		    " ***[%s]SUM|PCPU:%d,MEM:%ld,PROC:%d,OPENFILES:%d,RBPS:%d,WBPS:%d,RIOPS:%d,WIOPS:%d,PMEM:%d,TIME:%ld\n",
		    sumch->name, sumch->pcpu / round_total,
		    sumch->memoryuse / round_total,
		    sumch->maxproc / round_total,
		    sumch->openfiles / round_total,
		    sumch->readbps / round_total, sumch->writebps / round_total,
		    sumch->readiops / round_total,
		    sumch->writeiops / round_total, sumch->pmem / round_total,
		    sumch->modified / round_total);

		if (OUTPUT_BEANSTALKD & output_flags) {
			memset(json_buf, 0, sizeof(json_buf));
			sprintf(json_buf,
			    "{\"name\": \"%s\", \"time\": %d, \"pcpu\": %d, \"pmem\": %d,\"maxproc\": %d,\"openfiles\": %d,\"readbps\": %d,\"writebps\": %d,\"readiops\": %d,\"writeiops\": %d }",
			    sumch->name, cur_time, sumch->pcpu / round_total,
			    sumch->pmem / round_total,
			    sumch->maxproc / round_total,
			    sumch->openfiles / round_total,
			    sumch->readbps / round_total,
			    sumch->writebps / round_total,
			    sumch->readiops / round_total,
			    sumch->writeiops / round_total);

			if (strlen(json_str) > 2) {
				strcat(json_str, ",");
				strcat(json_str, json_buf);
			} else {
				strcpy(json_str,
				    "{ \"tube\":\"racct-jail\", \"data\":[");
				strcat(json_str, json_buf);
			}
		}
#ifdef WITH_INFLUX
		if (OUTPUT_INFLUX & output_flags) {
			//
			sprintf(influx->buffer + strlen(influx->buffer),
			    "%s,node=%s,host=%s%s%s memoryuse=%lu,maxproc=%d,openfiles=%d,pcpu=%d,readbps=%d,writebps=%d,readiops=%d,writeiops=%d,pmem=%d %lu\n",
			    influx->tables.jails, hostname, sumch->name,
			    (influx->tags.jails == NULL ? "" : ","),
			    (influx->tags.jails == NULL ? "" :
								influx->tags.jails),
			    sumch->memoryuse / round_total,
			    sumch->maxproc / round_total,
			    sumch->openfiles / round_total,
			    sumch->pcpu / round_total,
			    sumch->readbps / round_total,
			    sumch->writebps / round_total,
			    sumch->readiops / round_total,
			    sumch->writeiops / round_total,
			    sumch->pmem / round_total, nanoseconds());

			influx->items++;
			//			tolog(log_level,"%d RACCT items
			// queued for storage\n", influx->items);
		}
#endif

		if (OUTPUT_SQLITE3 & output_flags) {
			memset(sql, 0, sizeof(sql));
			memset(stats_file, 0, sizeof(stats_file));
			sprintf(stats_file, "%s/jails-system/%s/racct.sqlite",
			    workdir, sumch->name);
			fp = fopen(stats_file, "r");
			if (!fp) {
				tolog(log_level,
				    "RACCT not exist, create via updatesql\n");
				sprintf(sql,
				    "/usr/local/bin/cbsd /usr/local/cbsd/misc/updatesql %s /usr/local/cbsd/share/racct.schema racct",
				    stats_file);
				system(sql);
				// write into base in next loop (protection if
				// jail was removed in directory not exist
				// anymore
				continue;
			}
			fclose(fp);

			sprintf(sql,
			    "INSERT INTO racct ( idx,memoryuse,maxproc,openfiles,pcpu,readbps,writebps,readiops,writeiops,pmem ) VALUES ( '%d', '%lu', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d' );\n",
			    cur_time, sumch->memoryuse / round_total,
			    sumch->maxproc / round_total,
			    sumch->openfiles / round_total,
			    sumch->pcpu / round_total,
			    sumch->readbps / round_total,
			    sumch->writebps / round_total,
			    sumch->readiops / round_total,
			    sumch->writeiops / round_total,
			    sumch->pmem / round_total);
			tolog(log_level, "Save to SQL: %s [%s]\n", stats_file,
			    sql);
			ret = sqlitecmd(stats_file, sql);
		}

		sumch->modified = 0;
		sumch->pcpu = 0;
		sumch->memoryuse = 0;
		sumch->maxproc = 0;
		sumch->openfiles = 0;
		sumch->readbps = 0;
		sumch->writebps = 0;
		sumch->readiops = 0;
		sumch->writeiops = 0;
		sumch->pmem = 0;
		remove_data_by_jname(sumch->name);
	}

	if (OUTPUT_BEANSTALKD & output_flags) {
		strcat(json_str, "]}");
		bs_tick = 0;
	} else {
		skip_beanstalk = 1;
	}

	if (cur_round != save_loop_count || skip_beanstalk == 1) {
		return 0;
	}

	if (strlen(json_str) > 3) {
		tolog(log_level, "bs_put: (%s)\n", json_str);
		ret = bs_put(bs_socket, 0, 0, 0, json_str, strlen(json_str));
		if (ret > 0) {
			bs_tick = 1;
		} else {
			tolog(log_level,
			    "bs_put failed, trying to reconnect...\n");
			bs_disconnect(bs_socket);
			bs_connected = 0;
			return 1;
		}
	} else {
		tolog(log_level, "skip_beanstalk = 1,skipp\n");
	}
	return 0;
}

int
update_racct_jail(char *jname, char *orig_jname, int jid)
{
	struct item_data *target = NULL;
	struct item_data *ch;
	struct item_data *next_ch;
	struct timeval now_time;
	int cur_time = 0;
	int error;
	char *copy;
	char *outbuf = NULL;
	char *tmp;
	char *var;
	size_t outbuflen = RCTL_DEFAULT_BUFSIZE / 4;
	int store = 0;
	char param_name[512];
	char filter[MAXJNAME + 7];
	char unexpanded_rule[MAXJNAME + 7]; // 7 - extra "jail::\0"

	sprintf(filter, "jail:%s:", orig_jname);
	sprintf(unexpanded_rule, "jail:%s", orig_jname);

	gettimeofday(&now_time, NULL);
	cur_time = (time_t)now_time.tv_sec;

	for (ch = item_list; ch; ch = ch->next) {
		if (strcmp(jname, ch->name) == 0) {
			tolog(log_level, "update metrics for jail: [%s]\n",
			    jname);
			// ch->modified = (time_t) now_time.tv_sec;
			ch->modified = nanoseconds();
			ch->pid = cur_jid;

			for (;;) {
				outbuflen *= 4;
				outbuf = realloc(outbuf, outbuflen);
				if (outbuf == NULL) {
					err(1, "realloc");
				}
				error = rctl_get_racct(filter,
				    strlen(filter) + 1, outbuf, outbuflen);
				if (error == 0) {
					break;
				}
				if (errno == ERANGE) {
					continue;
				}
				if (errno == ENOSYS) {
					enosys();
				}

				warn(
				    "failed to show resource consumption for '%s'",
				    unexpanded_rule);
				free(outbuf);
				return (error);
			}
			copy = outbuf;
			int i = 0;
			while ((tmp = strsep(&copy, ",")) != NULL) {
				if (tmp[0] == '\0') {
					break; /* XXX */
				}

				while ((var = strsep(&tmp, "=")) != NULL) {
					i++;
					if (var[0] == '\0') {
						break; /* XXX */
					}
					if (i == 1) {
						memset(param_name, 0,
						    sizeof(param_name));
						strcpy(param_name, var);
					}
					if (i == 2) {
						// printf("val* %s\n",var);
						if (!strcmp(param_name,
							"cputime")) {
							ch->cputime = atoi(var);
						} else if (!strcmp(param_name,
							       "datasize")) {
							ch->datasize = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "stacksize")) {
							ch->stacksize = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "memoryuse")) {
							ch->memoryuse = atol(
							    var);
						} else if (
						    !strcmp(param_name,
							"memorylocked")) {
							ch->memorylocked = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "maxproc")) {
							ch->maxproc = atoi(var);
						} else if (!strcmp(param_name,
							       "openfiles")) {
							ch->openfiles = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "vmemoryuse")) {
							ch->vmemoryuse = atol(
							    var);
						} else if (!strcmp(param_name,
							       "swapuse")) {
							ch->swapuse = atoi(var);
						} else if (!strcmp(param_name,
							       "nthr")) {
							ch->nthr = atoi(var);
						} else if (!strcmp(param_name,
							       "readbps")) {
							ch->readbps = atoi(var);
						} else if (!strcmp(param_name,
							       "writebps")) {
							ch->writebps = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "readiops")) {
							ch->readiops = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "writeiops")) {
							ch->writeiops = atoi(
							    var);
						} else if (!strcmp(param_name,
							       "pcpu")) {
							if (ncpu > 1) {
								ch->pcpu =
								    (atoi(var) /
									ncpu);
							} else {
								ch->pcpu = atoi(
								    var);
							}
							if (ch->pcpu > 100) {
								ch->pcpu = 100;
							}
						} else {
							// calculate pmem
							ch->pmem = 100.0 *
							    ch->memoryuse /
							    maxmem;
							if (ch->pmem > 100) {
								ch->pmem = 100;
							}
						}
						i = 0;
					}
				}
			}
			free(outbuf);
		}
	}
	return 0;
}


// prom
/* Handle all communication with the client */
void *handle_client(void *arg){
	client_t *cli = (client_t *)arg;

/*
    char buff_out[BUFFER_SZ];
    char name[32];
    int leave_flag = 0;

    cli_count++;
    client_t *cli = (client_t *)arg;

    // Name
    if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
	printf("Didn't enter the name.\n");
	leave_flag = 1;
    } else{
	strcpy(cli->name, name);
	sprintf(buff_out, "%s has joined\n", cli->name);
	printf("%s", buff_out);
	send_message(buff_out, cli->uid);
    }

    bzero(buff_out, BUFFER_SZ);

    while(1){
	if (leave_flag) {
	    break;
	}

	int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
	if (receive > 0){
	    if(strlen(buff_out) > 0){
		send_message(buff_out, cli->uid);

		str_trim_lf(buff_out, strlen(buff_out));
		printf("%s -> %s\n", buff_out, cli->name);
	    }
	} else if (receive == 0 || strcmp(buff_out, "exit") == 0){
	    sprintf(buff_out, "%s has left\n", cli->name);
	    printf("%s", buff_out);
	    send_message(buff_out, cli->uid);
	    leave_flag = 1;
	} else {
	    printf("ERROR: -1\n");
	    leave_flag = 1;
	}

	bzero(buff_out, BUFFER_SZ);
    }
*/

  char s[2048];
  memset(s,0,strlen(s));


const char *content_encoding = "";

//            /* Gzip compress the output. */
//                if (gzip_mode) {
//                        char *buf;
//                        size_t buflen;
//        
//                        buflen = http_buflen;
//                        buf = malloc(buflen);
//                        if (buf == NULL)
//                                err(1, "Cannot allocate compression buffer");
//                        if (buf_gzip(http_buf, http_buflen, buf, &buflen)) {
//                                content_encoding = "Content-Encoding: gzip\r\n";
//                                free(http_buf);
//                                http_buf = buf;
//                                http_buflen = buflen;
//                        } else {
//                                free(buf);
//                        }
//                }

                /* Print HTTP header and metrics. */
sprintf(s,"\
HTTP/1.1 200 OK\r\n\
Connection: close\r\n\
%s\
Content-Type: text/plain; version=0.0.4\r\n\
\r\n",
    content_encoding);

 if(write(cli->sockfd, s, strlen(s)) < 0){
	perror("ERROR: write to descriptor failed");
//	break;
  }

////////////////
	struct item_data *target = NULL;
	struct item_data *ch;
	struct item_data *next_ch;
	char sql[512];
	char stats_file[1024];
	const char *hostname = getenv(
	    "HOST"); // Still banging the env every second or so, only do this
		     // at load?
	FILE *fp;
	char json_str[20000]; // todo: dynamic from number of bhyve/jails
	char json_buf[1024];  // todo: dynamic from number of bhyve/jails
	int i;
	struct timeval now_time;
	int cur_time = 0;
	int round_total = save_loop_count + 1;
	int jails_up=0;
	int jails_down=0;

	char dbfile[512];
	char query[100];

	sqlite3 *db;
	int ret = 0;
	sqlite3_stmt *stmt;
	int res = 0;

	struct sum_item_data *newd;
	struct sum_item_data *temp;
	struct sum_item_data *sumch;
	struct sum_item_data *next_sumch;

	tolog(log_level, "\n ***---calc jail avgdata---*** \n");

	gettimeofday(&now_time, NULL);
	cur_time = (time_t)now_time.tv_sec;

	for (ch = item_list; ch; ch = ch->next) {
		if (strlen(ch->orig_name) < 1) {
			continue;
		}
		if (ch->modified == 0) {
			continue;
		}
		i = sum_jname_exist(ch->orig_name);

		if (i) {
			for (sumch = sum_item_list; sumch;
			     sumch = sumch->next) {
				if (!strcmp(ch->orig_name, sumch->name)) {
					sumch->modified += ch->modified;
					sumch->pcpu += ch->pcpu;
					sumch->memoryuse += ch->memoryuse;
					sumch->maxproc += ch->maxproc;
					sumch->openfiles += ch->openfiles;
					sumch->readbps += ch->readbps;
					sumch->writebps += ch->writebps;
					sumch->readiops += ch->readiops;
					sumch->writeiops += ch->writeiops;
					sumch->pmem += ch->pmem;
					break;
				}
			}
		} else {
			CREATE(newd, struct sum_item_data, 1);
			newd->modified = ch->modified;
			newd->pcpu = ch->pcpu;
			newd->memoryuse = ch->memoryuse;
			newd->maxproc = ch->maxproc;
			newd->openfiles = ch->openfiles;
			newd->readbps = ch->readbps;
			newd->writebps = ch->writebps;
			newd->readiops = ch->readiops;
			newd->writeiops = ch->writeiops;
			newd->pmem = ch->pmem;
			newd->next = sum_item_list;
			sum_item_list = newd;
			strcpy(newd->name, ch->orig_name);
			tolog(log_level,
			    "[AVGSUM] !! %s struct has been added\n",
			    newd->name);
		}
	}

	memset(json_str, 0, sizeof(json_str));

	sprintf(json_str,"\
jails_up: %d\n\
", jails_up);

	for (sumch = sum_item_list; sumch; sumch = sumch->next) {
		if (strlen(sumch->name) < 1) {
			continue;
		}

		sprintf(json_str,"\
jail_openfiles{name=\"%s\"} %d\n\
jail_memoryuse{name=\"%s\"} %lu\n\
jail_maxproc{name=\"%s\"} %d\n\
jail_readbps{name=\"%s\"} %d\n\
jail_writebps{name=\"%s\"} %d\n\
jail_readiops{name=\"%s\"} %d\n\
jail_writeiops{name=\"%s\"} %d\n\
jail_pcpu{name=\"%s\"} %d\n\
", sumch->name,sumch->openfiles / round_total,
sumch->name,sumch->memoryuse / round_total,
sumch->name,sumch->maxproc / round_total,
sumch->name,sumch->readbps / round_total,
sumch->name,sumch->writebps / round_total,
sumch->name,sumch->readiops / round_total,
sumch->name,sumch->writeiops / round_total,
sumch->name,sumch->pcpu / round_total );

  if(write(cli->sockfd, json_str, strlen(json_str)) < 0){
	perror("ERROR: write to descriptor failed");
//	break;
    }


//			sprintf(json_str,
//			    "INSERT INTO racct ( idx,memoryuse,maxproc,openfiles,pcpu,readbps,writebps,readiops,writeiops,pmem ) VALUES ( '%d', '%lu', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d' );\n",
//			    cur_time, sumch->memoryuse / round_total,
//			    sumch->maxproc / round_total,
//			    sumch->openfiles / round_total,
//			    sumch->pcpu / round_total,
//			    sumch->readbps / round_total,
//			    sumch->writebps / round_total,
//			    sumch->readiops / round_total,
//			    sumch->writeiops / round_total,
//			    sumch->pmem / round_total);
		jails_up=jails_up+1;
		}

	memset(json_str, 0, sizeof(json_str));

	sprintf(json_str,"\
jails_up: %d\n\
", jails_up);

  if(write(cli->sockfd, json_str, strlen(json_str)) < 0){
	perror("ERROR: write to descriptor failed");
//	break;
    }


	//offline
	memset(dbfile, 0, sizeof(dbfile));
	sprintf(dbfile, "%s/var/db/local.sqlite", workdir);

	if (SQLITE_OK != (res = sqlite3_open(dbfile, &db))) {
		tolog(log_level, "%s: Can't open database file: %s\n", nm(), dbfile);
	} else {
		res = 1024;

		sprintf(query, "SELECT COUNT(jname) FROM jails WHERE emulator=\"jail\" AND status='0'");
		ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

		if (ret == SQLITE_OK) {
			ret = sqlite3_step(stmt);

			while (ret == SQLITE_ROW) {
				jails_down = sql_get_int(stmt);
				ret = sqlite3_step(stmt);
			}
		}

		sqlite3_finalize(stmt);
		sqlite3_close(db);

		sprintf(json_str,"\
jails_down: %d\n\
", jails_down);


		if(write(cli->sockfd, json_str, strlen(json_str)) < 0){
			perror("ERROR: write to descriptor failed");
		}

	memset(json_str, 0, sizeof(json_str));
	sprintf(json_str,"cbsd_pool_info{nodename=\"%s\"} 1\n", pool_name);

	if(write(cli->sockfd, json_str, strlen(json_str)) < 0){
		perror("ERROR: write to descriptor failed");
	}


	}

////////////////


// if(write(cli->sockfd, json_str, strlen(json_str)) < 0){
//	perror("ERROR: write to descriptor failed");
//	break;
//    }



  /* Delete client from queue and yield thread */
    close(cli->sockfd);
//  queue_remove(cli->uid);
  free(cli);
//  cli_count--;
//  pthread_detach(pthread_self());

    pthread_exit(NULL);

    return 0;
}
//


// prom
/* Handle all communication with the client */
void *handle_accept() {
	int connfd=0;
	int tid;
	int total = 1;
	int curThread;
	pthread_t threads[total];

	tolog(log_level,"thread #%ld, handle accept\n",tid);

//// prom
	socklen_t clilen = sizeof(cli_addr);
	connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

	/* Check if max clients is reached */
/*
	if((cli_count + 1) == MAX_CLIENTS){
	    printf("Max clients reached. Rejected: ");
	    print_client_addr(cli_addr);
	    printf(":%d\n", cli_addr.sin_port);
	    close(connfd);
	    continue;
	}
*/

	/* Client settings */
	client_t *cli = (client_t *)malloc(sizeof(client_t));
	cli->address = cli_addr;
	cli->sockfd = connfd;
//	cli->uid = uid++;

	/* Add client to the queue and fork thread */
//	queue_add(cli);
	for (curThread = 0; curThread < total; curThread++){
		tid=curThread;
		tolog(log_level,"* run handle_client thread #%d\n",curThread);
		if (pthread_create(&threads[curThread], NULL, handle_client, (void*)cli)) {
			tolog(log_level,"Error creating thread %i of %i\n", curThread, total);
			exit(1);
		}
	}

	for (curThread = 0; curThread < total; curThread++){
		tolog(log_level,"* waiting #%d\n",curThread);
		if (pthread_join(threads[curThread], NULL)) {
			tolog(log_level,"Error waiting for thread %i of %i\n", curThread, total);
			exit(2);
		}
	}
// prom

	accept_busy=0;
	tolog(log_level,"reset accept_busy\n");
//	pthread_detach(pthread_self());
	pthread_exit(NULL);
}
//






int
main(int argc, char **argv)
{
	char *dot;
	char *ep;
	char *jname;
	char *pname;
	int c;
	int i;
	int jflags;
	int jid;
	int lastjid;
	int pflags;
	int spc;
	struct item_data *newd;
	struct item_data *temp;
	struct timeval now_time;
	size_t ncpu_len = 0;
	size_t maxmem_len = 0;
	DIR *dirp = NULL;
	char *yaml;
	int current_jobs_ready = 0;
	int jobs_max = 2; // jobs_max per one item, one graph = 25 rec
	int jobs_max_all_items = 0;
	int current_waiting = 0;
	BSJ *job;
	int x;
	char rnum[5];
	int optcode = 0;
	int option_index = 0;
	int tid;

	struct dirent *dp;
	char vmname[MAXJNAME];
	char vmpath[MAXJNAME];
	char tmpjname[MAXJNAME];
	pid_t vmpid;
	char jailpath[512];

	FILE *fp;

	char *ip;

	int total = 1;
	int v6 = 0;
	int curThread;
	pthread_t threads[total];

	static struct option long_options[] = { { "help", no_argument, 0,
						    C_HELP },
		{ "log_file", required_argument, 0, C_LOG_FILE },
		{ "pool_name", required_argument, 0, C_POOL_NAME },
		{ "log_level", required_argument, 0, C_LOG_LEVEL },
		{ "loop_interval", required_argument, 0, C_LOOP_INTERVAL },
		{ "prometheus_exporter", required_argument, 0,
		    C_PROMETHEUS_EXPORTER },
		{ "prometheus_listen4", required_argument, 0,
		    C_PROMETHEUS_LISTEN4 },
		{ "prometheus_listen6", required_argument, 0,
		    C_PROMETHEUS_LISTEN6 },
		{ "prometheus_port", required_argument, 0,
		    C_PROMETHEUS_PORT },
		{ "save_loop_count", required_argument, 0, C_SAVE_LOOP_COUNT },
		{ "save_beanstalkd", required_argument, 0, C_SAVE_BEANSTALKD },
		{ "save_sqlite3", required_argument, 0, C_SAVE_SQLITE3 },
#ifdef WITH_INFLUX
		{ "save_influx", required_argument, 0, C_SAVE_INFLUX },
#endif
		/* End of options marker */
		{ 0, 0, 0, 0 } };

	while (TRUE) {
		optcode = getopt_long_only(argc, argv, "", long_options,
		    &option_index);
		if (optcode == -1) {
			break;
		}
		switch (optcode) {
		case C_HELP:
			usage();
			break;
		case C_LOG_FILE:
			log_file = optarg;
			break;
		case C_POOL_NAME:
			pool_name = optarg;
			break;
		case C_LOG_LEVEL:
			log_level = atoi(optarg);
			break;
		case C_LOOP_INTERVAL:
			loop_interval = atoi(optarg);
			break;
		case C_PROMETHEUS_EXPORTER:
			if (atoi(optarg) == 1) {
				output_flags |= OUTPUT_PROMETHEUS;
			}
			break;
		case C_PROMETHEUS_LISTEN4:
			prometheus_listen4 = optarg;
			break;
		case C_PROMETHEUS_LISTEN6:
			prometheus_listen6 = optarg;
			break;
		case C_PROMETHEUS_PORT:
			prometheus_port = atoi(optarg);
			break;
		case C_SAVE_LOOP_COUNT:
			save_loop_count = atoi(optarg);
			break;
		case C_SAVE_BEANSTALKD:
			if (atoi(optarg) == 1) {
				output_flags |= OUTPUT_BEANSTALKD;
			}
			break;
		case C_SAVE_SQLITE3:
			if (atoi(optarg) == 1) {
				output_flags |= OUTPUT_SQLITE3;
			}
			break;
#ifdef WITH_INFLUX
		case C_SAVE_INFLUX:
			if (atoi(optarg) == 1)
				output_flags |= OUTPUT_INFLUX;
			break;
#endif
		}
	}

	printf("CBSD jail racct statistics exporter\n");
	if (log_file) {
		printf("log_file: %s\n", log_file);
	}
	if(!pool_name) {
		//gethostbyname
		pool_name="unknown";
	}

	printf("pool_name: %s\n", pool_name);

	printf("log_level: %d\n", log_level);
	printf("loop_interval: %d seconds\n", loop_interval);
	printf("save_loop_count: %d\n", save_loop_count);
	printf("beanstalkd enabled: %s\n",
	    (OUTPUT_BEANSTALKD & output_flags) ? "yes" : "no");
	printf("prometheus enabled: %s\n",
	    (OUTPUT_PROMETHEUS & output_flags) ? "yes" : "no");
	printf("sqlite3 enabled: %s\n",
	    (OUTPUT_SQLITE3 & output_flags) ? "yes" : "no");
#ifdef WITH_INFLUX
	printf("influx enabled: %s\n",
	    (OUTPUT_INFLUX & output_flags) ? "yes" : "no");
#endif

	if (output_flags == 0) {
#ifdef WITH_INFLUX
		printf(
		    "Error: select at least one backend ( --prometheus_exported | --save_beanstalkd | --save_sqlite3 | --save_influx )\n");
#else
		printf(
		    "Error: select at least one backend ( --prometheus_exported | --save_beanstalkd | --save_sqlite3 )\n");
#endif
		exit(-1);
	}

	ncpu_len = sizeof(ncpu);
	maxmem_len = sizeof(maxmem);

	jname = NULL;
	pflags = jflags = jid = 0;

	int pipe_fd[2];
	pid_t otherpid;
	char name[] = "racct-jail-statsd";

	workdir = getenv("workdir");

	if (workdir == NULL) {
		printf("no workdir env\n");
		exit(1);
	}

	if (pipe(pipe_fd) == -1) {
		printf("pipe");
		exit(-1);
	}

	switch (fork()) {
	case -1:
		printf("cannot fork");
		exit(-1);
	case 0:
		break;
	default:
		return (0);
	}

	// fork
	setproctitle("%s", name);

	setsid();
	dup2(nullfd, STDIN_FILENO);
	dup2(nullfd, STDOUT_FILENO);
	dup2(nullfd, STDERR_FILENO);
	close(nullfd);
	// close(pipe_fd[0]);
	// close(pipe_fd[1]);

	if (path_my_pidfile == NULL) {

		asprintf(&path_my_pidfile, "%s/var/run/cbsd_statsd_jail.pid",
			workdir);
//		asprintf(&path_my_pidfile, "%sracct-jail-statsd.pid",
//			_PATH_VARRUN);
		if (path_my_pidfile == NULL) {
			printf("asprintf");
			exit(1);
		}
	}
	pidfile = pidfile_open(path_my_pidfile, 0644, &otherpid);
	if (pidfile == NULL) {
		if (errno == EEXIST) {
			printf("racct-jail-statsd already running, pid: %d.",
			    otherpid);
			exit(1);
		}
		if (errno == EAGAIN) {
			printf("racct-jail-statsd already running.");
			exit(1);
		}
		printf("Cannot open or create pidfile: %s", path_my_pidfile);
	}

	if (pidfile != NULL) {
		pidfile_write(pidfile);
	}

	i = sysctlbyname("hw.physmem", &maxmem, &maxmem_len, NULL, 0);

	if (i != 0) {
		if (errno == ENOENT) {
			errx(1,
			    "Unable to determine hoster physical memory via sysctl hw.physmem");
		}
		err(1, "sysctlbyname");
	}

	i = sysctlbyname("hw.ncpu", &ncpu, &ncpu_len, NULL, 0);

	if (i != 0) {
		if (errno == ENOENT) {
			errx(1, "Unable to determine CPU count via hw.ncpu");
		}
		err(1, "sysctlbyname");
	}

	/* Add the parameters to print. */
	add_param("jid", NULL, (size_t)0, NULL, JP_USER);
	add_param("name", NULL, (size_t)0, NULL, JP_USER);
	add_param("lastjid", &lastjid, sizeof(lastjid), NULL, 0);

	c = 0;

#ifdef WITH_INFLUX
	cbsd_influx_init();
	load_config();
#endif

/////////// prom
	if(prometheus_listen6) {
		ip = prometheus_listen6;
		v6 = 1;
	} else if(prometheus_listen4) {
		ip = prometheus_listen4;
		v6 = 0;
	} else {
		printf("No --prometheus_listen4 or prometheus_listen6\n");
	}

	printf("Prometheus listen on: [%s:%d]\n",ip, prometheus_port);

  int port = prometheus_port;
  int option = 1;

  struct sockaddr_in serv_addr4;
  struct sockaddr_in6 serv_addr6;

/* Socket settings */
if ( v6 == 0 ) {
	// v4
	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	serv_addr4.sin_family = AF_INET;
	serv_addr4.sin_addr.s_addr = inet_addr(ip);
	serv_addr4.sin_port = htons(port);
} else {
	// v6
	listenfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	serv_addr6.sin6_family = AF_INET6;
	serv_addr6.sin6_addr = in6addr_any;
	serv_addr6.sin6_port = htons(port);
}

  /* Ignore pipe signals */
  signal(SIGPIPE, SIG_IGN);

//    if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT_LB,(char*)&option,sizeof(option)) < 0){
	perror("ERROR: setsockopt failed");
	exit(0);
//    return EXIT_FAILURE;
    }

    /* Bind */
if ( v6 == 0 ) {
	if(bind(listenfd, (struct sockaddr*)&serv_addr4, sizeof(serv_addr4)) < 0) {
		perror("ERROR: Socket v4 binding failed");
		close(listenfd);
		exit(1);
	}
} else {
	if(bind(listenfd, (struct sockaddr*)&serv_addr6, sizeof(serv_addr6)) < 0) {
		perror("ERROR: Socket v6 binding failed");
		close(listenfd);
		exit(1);
	}
}

  /* Listen */
  if (listen(listenfd, 10) < 0) {
    perror("ERROR: Socket listening failed");
	exit(0);
//    return EXIT_FAILURE;
    }
//////////////// prom


	running_jails = 0;
	while (1) {
		tolog(log_level, "main loop\n");

		if ((OUTPUT_BEANSTALKD & output_flags) && bs_connected != 1) {
			if (bs_socket != -1) {
				bs_disconnect(bs_socket);
			}
			bs_socket = init_bs("racct-jail");
		} else if (!(OUTPUT_BEANSTALKD & output_flags)) {
			bs_connected = 0;
		}

		while (true) {
			if ((OUTPUT_BEANSTALKD & output_flags) &&
			    bs_connected != 1) {
				break;
			}

#ifdef WITH_INFLUX
			if (OUTPUT_INFLUX & output_flags) {
				if (influx->items > (4 * (1 + running_jails)) ||
				    influx->items >
					50) { // 50 rows max to be sure we don't
					      // overflow the buffer.
					int rc;
					if ((rc = cbsd_influx_transmit_buffer()) !=
					    0)
						tolog(log_level,
						    "RACCT to Influx failed! [code:%d]\n",
						    rc);
				}
			}
#endif

			if (accept_busy==0) {
				for (curThread = 0; curThread < total; curThread++){
					tid=curThread;
					tolog(log_level,"* run accept thread #%d\n",curThread);
					if (pthread_create(&threads[curThread], NULL, handle_accept, &total)) {
						tolog(log_level,"Error creating accept thread %i of %i\n", curThread, total);
						exit(1);
					}
					tolog(log_level,"accept thread created\n");
				}
				accept_busy=1;
			}
//			if (pthread_join(threads[1], NULL)) {
//				tolog(log_level,"Error waiting for thread 1");
//			}

			tolog(log_level, " round %d/%d\n ---------------- \n",
			    cur_round, save_loop_count);
			// convert round integer to string
			memset(rnum, 0, sizeof(rnum));
			sprintf(rnum, "%d", cur_round);
			// jail area
			running_jails = 0;
			for (lastjid = 0; lastjid >= 0;) {
				memset(cur_jname, 0, sizeof(cur_jname));
				lastjid = print_jail(pflags, jflags);
				if (cur_jid == 0) {
					continue;
				}
				if (strlen(cur_jname) < 1) {
					continue;
				}

				memset(tmpjname, 0, sizeof(tmpjname));
				strcpy(tmpjname, cur_jname);

				//check if jails outside the CBSD workdir
				memset(jailpath,0,sizeof(jailpath));
				sprintf(jailpath, "%s/jails-data/%s-data", workdir,tmpjname);
				fp=fopen(jailpath,"r");
				if (!fp) {
					tolog(log_level,"[JAIL] skipp: %s jail do not belong to [%s] workdir\n", tmpjname, workdir);
					continue;
				} else {
					fclose(fp);
				}

				cur_jname[strlen(cur_jname)] = '\0';
				strcat(cur_jname, "_");
				strcat(cur_jname, rnum);
				cur_jname[strlen(cur_jname)] = '\0';

				i = jname_exist(cur_jname);

				if (i) {
					running_jails++;
					update_racct_jail(cur_jname, tmpjname,
					    cur_jid);
					continue;
				}
				CREATE(newd, struct item_data, 1);
				newd->cputime = lastjid;
				newd->pid = cur_jid;
				newd->modified = 0; // sign of new jail
				newd->next = item_list;
				item_list = newd;
				strcpy(newd->name, cur_jname);
				strcpy(newd->orig_name, tmpjname);
				tolog(log_level,
				    "[JAIL] !! %d [%s (%s)] has beed added\n",
				    cur_jid, cur_jname, tmpjname);
			}

			c++;
			list_data();

			if (c > 5) {
				prune_inactive_env();
				c = 0;
			}

			if (OUTPUT_BEANSTALKD & output_flags) {
				i = bs_stats_tube(bs_socket, "racct-jail",
				    &yaml);
				if (yaml) {
					current_jobs_ready = get_bs_stats(yaml,
					    current_jobs_ready_str);
					current_waiting = get_bs_stats(yaml,
					    current_waiting_str);
					//					current_jobs_ready=get_bs_stats(yaml,"current-jobs-ready:
					//");
					//					current_waiting=get_bs_stats(yaml,"current-waiting:
					//");
					free(yaml);
					if (current_jobs_ready < 0) {
						tolog(log_level,
						    "get_bs_stats failed for current-jobs-ready\n");
						bs_connected = 0;
						sleep(loop_interval);
						break;
					}
					if (current_waiting < 0) {
						tolog(log_level,
						    "get_bs_stats failed for current-waiting\n");
						bs_connected = 0;
						sleep(loop_interval);
						break;
					}
					tolog(log_level,
					    "current-jobs: %d, jobs_max_all: %d, current-waiting: %d\n",
					    current_jobs_ready,
					    jobs_max_all_items,
					    current_waiting);
				} else {
					current_waiting = -1;
					current_jobs_ready = -1;
					bs_connected = 0;
					tolog(log_level,
					    "bs_stats_tube yaml error,reset bs connection\n");
					sleep(1);
					break;
				}

				skip_beanstalk = 1;
				if (current_waiting == 0) {
					// no consumer, (flush old data?)
					tolog(log_level,
					    "[debug]no waiting consumer anymore, clear/flush old jobs: %d\n",
					    current_jobs_ready);
					//					for
					//(i=0;i<current_jobs_ready;i++) {
					////remove
					//						bs_reserve_with_timeout(bs_socket,
					// 1, &job);
					// bs_release(bs_socket, job->id, 0, 0);
					// bs_free_job(job);
					//						bs_peek_ready(bs_socket,
					//&job);
					//bs_delete(bs_socket, job->id);
					//						bs_free_job(job);
					//					}
				} else if (current_jobs_ready > 20) {
					skip_beanstalk = 1;
					tolog(log_level,
					    "[debug]too many ready jobs in bs: %d. skip for beanstalk\n",
					    current_jobs_ready);
				} else {
					skip_beanstalk = 0;
				}
			}

			// giant cycle sleep
			tolog(log_level, "\n");

			sleep(loop_interval);
			cur_round++;
			if (cur_round > save_loop_count) {
				cur_round = 0;
			}
			if (cur_round == save_loop_count) {
				sum_data();
			}
		}
	}

	if (pidfile != NULL) {
		pidfile_remove(pidfile);
	}

#ifdef WITH_INFLUX
	cbsd_influx_free();
#endif

	return 0;
}

static int
add_param(const char *name, void *value, size_t valuelen,
    struct jailparam *source, unsigned flags)
{
	struct jailparam *param;
	struct jailparam *tparams;
	int i;
	int tnparams;

	static int paramlistsize;

	/* The pseudo-parameter "all" scans the list of available parameters. */
	if (!strcmp(name, "all")) {
		tnparams = jailparam_all(&tparams);
		if (tnparams < 0) {
			printf("error: %s", jail_errmsg);
			return 1;
		}
		qsort(tparams, (size_t)tnparams, sizeof(struct jailparam),
		    sort_param);

		for (i = 0; i < tnparams; i++) {
			add_param(tparams[i].jp_name, NULL, (size_t)0,
			    tparams + i, flags);
		}

		free(tparams);
		return -1;
	}

	/* Check for repeat parameters. */
	for (i = 0; i < nparams; i++) {
		if (!strcmp(name, params[i].jp_name)) {
			if (value != NULL &&
			    jailparam_import_raw(params + i, value, valuelen) <
				0) {
				printf("error: %s", jail_errmsg);
				return 1;
			}
			params[i].jp_flags |= flags;

			if (source != NULL) {
				jailparam_free(source, 1);
			}
			return i;
		}
	}

	/* Make sure there is room for the new param record. */
	if (!nparams) {
		paramlistsize = 32;
		params = malloc(paramlistsize * sizeof(*params));
		param_parent = malloc(paramlistsize * sizeof(*param_parent));
		if (params == NULL || param_parent == NULL) {
			printf("malloc");
			return 1;
		}
	} else if (nparams >= paramlistsize) {
		paramlistsize *= 2;
		void *tmp_params = realloc(params,
		    paramlistsize * sizeof(*params));
		void *tmp_param_parent = realloc(param_parent,
		    paramlistsize * sizeof(*param_parent));
		if (tmp_params == NULL || tmp_param_parent == NULL) {
			printf("realloc");
			free(params);
			free(param_parent);
			return 1;
		}
		params = tmp_params;
		param_parent = tmp_param_parent;
	}

	/* Look up the parameter. */
	param_parent[nparams] = -1;
	param = params + nparams++;
	if (source != NULL) {
		*param = *source;
		param->jp_flags |= flags;
		return param - params;
	}
	if (jailparam_init(param, name) < 0 ||
	    (value != NULL ? jailparam_import_raw(param, value, valuelen) :
				   jailparam_import(param, value)) < 0) {
		if (flags & JP_OPT) {
			nparams--;
			return (-1);
		}
		printf("error: %s", jail_errmsg);
		return 1;
	}
	param->jp_flags = flags;
	return param - params;
}

static int
sort_param(const void *a, const void *b)
{
	const struct jailparam *parama;
	const struct jailparam *paramb;
	char *ap;
	char *bp;

	/* Put top-level parameters first. */
	parama = a;
	paramb = b;
	ap = strchr(parama->jp_name, '.');
	bp = strchr(paramb->jp_name, '.');
	if (ap && !bp) {
		return (1);
	}
	if (bp && !ap) {
		return (-1);
	}

	return (strcmp(parama->jp_name, paramb->jp_name));
}

static int
print_jail(int pflags, int jflags)
{
	char *nname;
	char **param_values;
	int i;
	int ai;
	int jid;
	int count;
	int n;
	int spc;

	jid = jailparam_get(params, nparams, jflags);

	if (jid < 0) {
		return jid;
	}

	cur_jid = *(int *)params[0].jp_value;
	strcpy(cur_jname, (char *)params[1].jp_value);

	return jid;
}

int
list_data()
{
	struct item_data *target = NULL;
	struct item_data *ch;
	struct item_data *next_ch;
	int ret = 0;

	if (log_level == 0) {
		return 0;
	}

	tolog(log_level, "---listdata---\n");

	for (ch = item_list; ch; ch = ch->next) {
		if (ch->modified == 0) {
			continue;
		}
		tolog(log_level,
		    "TIME:%ld,NAME:%s,ORIGNAME:%s,PID:%d,PCPU:%d,MEM:%lu,PROC:%d,OPENFILES:%d,RB:%d,WB:%d,RIO:%d,WIO:%d,PMEM:%d\n",
		    ch->modified, ch->name, ch->orig_name, ch->pid, ch->pcpu,
		    ch->memoryuse, ch->maxproc, ch->openfiles, ch->readbps,
		    ch->writebps, ch->readiops, ch->writeiops, ch->pmem);
	}

	return 0;
}
