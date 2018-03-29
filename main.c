#include "main.h"

int app_state = APP_INIT;

TSVresult config_tsv = TSVRESULT_INITIALIZER;
char *db_data_path;

int sock_port = -1;
int sock_fd = -1;
int sock_fd_tf = -1;
Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};

PeerList peer_list = LIST_INITIALIZER;
SensorFTSList sensor_fts_list = LIST_INITIALIZER;
GroupList group_list = LIST_INITIALIZER;

#include "util.c"
#include "db.c"

int readSettings(TSVresult* r, const char *data_path, int *port, char **db_data_path) {
    if (!TSVinit(r, data_path)) {
        return 0;
    }
    int _port = TSVgetis(r, 0, "port");
    char *_db_data_path = TSVgetvalues(r, 0, "db_data_path");
    if (TSVnullreturned(r)) {
        return 0;
    }
    *port = _port;
    *db_data_path = _db_data_path;
    return 1;
}

void initApp() {
    if (!readSettings(&config_tsv, CONFIG_FILE, &sock_port, &db_data_path)) {
        exit_nicely_e("initApp: failed to read settings\n");
    }
#ifdef MODE_DEBUG
    printf("%s(): \n\tsock_port: %d, \n\tdb_data_path: %s\n", F, sock_port, db_data_path);
#endif
    if (!initServer(&sock_fd, sock_port)) {
        exit_nicely_e("initApp: failed to initialize udp server\n");
    }
    if (!initClient(&sock_fd_tf, WAIT_RESP_TIMEOUT)) {
        exit_nicely_e("initApp: failed to initialize udp client\n");
    }
}

int initData() {
    if (!config_getPeerList(&peer_list, &sock_fd_tf, db_data_path)) {
        freePeerList(&peer_list);
        return 0;
    }
    if (!config_getSensorFTSList(&sensor_fts_list, &peer_list, db_data_path)) {
        FREE_LIST(&sensor_fts_list)
        freePeerList(&peer_list);
        return 0;
    }
    if (!getGroupList(&group_list, db_data_path)) {
        FREE_LIST(&sensor_fts_list)
        freePeerList(&peer_list);
        return 0;
    }
    if (!prepGroupList(&group_list, &sensor_fts_list)) {
        freeGroupList();
        FREE_LIST(&sensor_fts_list)
        freePeerList(&peer_list);
        return 0;
    }
    return 1;
}
#define PARSE_I1LIST acp_requestDataToI1List(&request, &i1l);if (i1l.length <= 0) {return;}

void serverRun(int *state, int init_state) {
    SERVER_HEADER
    SERVER_APP_ACTIONS
    DEF_SERVER_I1LIST
    if (ACP_CMD_IS(ACP_CMD_GET_FTS)) {
        PARSE_I1LIST
        for (int i = 0; i < i1l.length; i++) {
            Group *item = getGroupById(i1l.item[i], &group_list);
            if (item != NULL) {
                readFTS(item, &response);
            }
        }
    }

    acp_responseSend(&response, &peer_client);
}

void freeData() {
    freeGroupList();
    FREE_LIST(&sensor_fts_list);
    freePeerList(&peer_list);
#ifdef MODE_DEBUG
    puts("freeData: done");
#endif
}

void freeApp() {
    freeData();
    freeSocketFd(&sock_fd);
    freeSocketFd(&sock_fd_tf);
    TSVclear(&config_tsv);
}

void exit_nicely() {

    freeApp();
#ifdef MODE_DEBUG
    puts("\nBye...");
#endif
    exit(EXIT_SUCCESS);
}

void exit_nicely_e(char *s) {
#ifdef MODE_DEBUG

    fprintf(stderr, "%s", s);
#endif
    freeApp();
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
#ifndef MODE_DEBUG
    daemon(0, 0);
#endif
    conSig(&exit_nicely);
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("main: memory locking failed");
    }
    int data_initialized = 0;
    while (1) {
#ifdef MODE_DEBUG
        printf("%s(): %s %d\n", F, getAppState(app_state), data_initialized);
#endif
        switch (app_state) {
            case APP_INIT:
                initApp();
                app_state = APP_INIT_DATA;
                break;
            case APP_INIT_DATA:
                data_initialized = initData();
                app_state = APP_RUN;
                delayUsIdle(1000000);
                break;
            case APP_RUN:
                serverRun(&app_state, data_initialized);
                break;
            case APP_STOP:
                freeData();
                data_initialized = 0;
                app_state = APP_RUN;
                break;
            case APP_RESET:
                freeApp();
                delayUsIdle(1000000);
                data_initialized = 0;
                app_state = APP_INIT;
                break;
            case APP_EXIT:
                exit_nicely();
                break;
            default:
                exit_nicely_e("main: unknown application state");
                break;
        }
    }
    freeApp();
    return (EXIT_SUCCESS);
}