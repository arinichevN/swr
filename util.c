
#include "main.h"

FUN_LIST_GET_BY_ID(Group)
FUN_RLIST_PUSH(Success)
FUN_RLIST_INIT(Success)

#define GLIi group_list->item[i]
#define GLIpi group_list.item[i]
#define SLIk sensor_list.item[k]
int prepGroupList(GroupList *group_list, SensorFTSList *sensor_fts_list) {
    for (int i = 0; i < group_list->length; i++) {
        for (int k = 0; k < GLIi.sensor_list.length; k++) {
            GLIi.SLIk.sensor_fts = getSensorFTSById(GLIi.SLIk.id, sensor_fts_list);
            if (GLIi.SLIk.sensor_fts != NULL) {
                GLIi.SLIk.fts = &GLIi.SLIk.sensor_fts->value;
            }
            if (!initSuccessRList(&GLIi.SLIk.result_list, GLIi.sensor_rank_size)) {
#ifdef MODE_DEBUG
                fprintf(stderr, "%s(): failed to initialize SuccessRList where group_id=%d and sensor_id=%d\n",__func__, GLIi.id, GLIi.SLIk.id);
#endif
            }
            GLIi.SLIk.success_count = 0;
            GLIi.SLIk.used = 0;
        }

    }
    return 1;
}

void freeGroupList() {
    extern GroupList group_list;
    for (int i = 0; i < group_list.length; i++) {
        for (int k = 0; k < GLIpi.sensor_list.length; k++) {
            FREE_RLIST(&GLIpi.SLIk.result_list)
        }
        FREE_LIST(&GLIpi.sensor_list)
    }
    FREE_LIST(&group_list)
}

void estimateSensor(Sensor *item, int result) {
    if (result == SUCCESS) {
        pushSuccessRList(&item->result_list, 1);
    } else if (result == FAILURE) {
        pushSuccessRList(&item->result_list, 0);
    } else {
        return;
    }
    item->success_count = 0;
    for (int i = 0; i < item->result_list.length; i++) {
        if (item->result_list.item[i] == 1) {
            item->success_count++;
        }
    }
}

Sensor * getNextSensor(Group *group) {
    int best = -1;
    Sensor *item = NULL;
    for (int i = 0; i < group->sensor_list.length; i++) {
        if (!group->sensor_list.item[i].used && group->sensor_list.item[i].success_count > best) {
            item = group->sensor_list.item + i;
            best = group->sensor_list.item[i].success_count;
        }
    }
    if (item != NULL) {
        item->used = 1;
    }
    return item;
}

void rewindSensorList(Group *group) {
    for (int i = 0; i < group->sensor_list.length; i++) {
        group->sensor_list.item[i].used = 0;
    }
}

int readFTS(Group *item, ACPResponse *response) {
#ifdef MODE_DEBUG
    printf("reading for group with id=%d:\n", item->id);
#endif
    while (1) {
        Sensor *sensor = getNextSensor(item);
        if (sensor != NULL) {
#ifdef MODE_DEBUG
            printf("\ttrying to read from sensor with id=%d:\n", sensor->id);
#endif
            if (acp_readSensorFTS(sensor->sensor_fts)) {
                estimateSensor(sensor, SUCCESS);
                rewindSensorList(item);
#ifdef MODE_DEBUG
                printf("\t\tsuccess\n");
#endif
                return acp_responseFTSCat(item->id, sensor->fts->value, sensor->fts->tm, sensor->fts->state, response);
            } else {
#ifdef MODE_DEBUG
                printf("\t\tfailure\n");
#endif
                estimateSensor(sensor, FAILURE);
            }
        } else {
            break;
        }
    }
#ifdef MODE_DEBUG
    printf("\tfailed\n");
#endif
    rewindSensorList(item);
    struct timespec tm = getCurrentTime();
    return acp_responseFTSCat(item->id, 0.0f, tm, 0, response);
}

void printData(ACPResponse *response) {
    extern GroupList group_list;
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "CONFIG_FILE: %s\n", CONFIG_FILE);
    SEND_STR(q)
    snprintf(q, sizeof q, "port: %d\n", sock_port);
    SEND_STR(q)
    snprintf(q, sizeof q, "db_data_path: %s\n", db_data_path);
    SEND_STR(q)
    snprintf(q, sizeof q, "db_public_path: %s\n", db_public_path);
    SEND_STR(q)
    snprintf(q, sizeof q, "app_state: %s\n", getAppState(app_state));
    SEND_STR(q)
    snprintf(q, sizeof q, "PID: %d\n", getpid());
    SEND_STR(q)
    SEND_STR("+-----------------------+\n")
    SEND_STR("|          Group        |\n")
    SEND_STR("+-----------+-----------+\n")
    SEND_STR("|    id     |snr_rank_sz|\n")
    SEND_STR("+-----------+-----------+\n")
    for (int i = 0; i < group_list.length; i++) {
        snprintf(q, sizeof q, "|%11d|%11u|\n",
                GLIpi.id,
                GLIpi.sensor_rank_size
                );
        SEND_STR(q)
    }
    SEND_STR("+-----------+-----------+\n")

    SEND_STR("+------------------------------------------------\n")
    SEND_STR("|                  Group sensor                  \n")
    SEND_STR("+-----------+-----------+-----------+------------\n")
    SEND_STR("|  group_id | sensor_id |success_cnt|result_list \n")
    SEND_STR("+-----------+-----------+-----------+------------\n")
    for (int i = 0; i < group_list.length; i++) {
        for (int k = 0; k < GLIpi.sensor_list.length; k++) {
            snprintf(q, sizeof q, "|%11d|%11d|%11u|",
                    GLIpi.id,
                    GLIpi.SLIk.id,
                    GLIpi.SLIk.success_count
                    );
            SEND_STR(q)
            for (int h = 0; h < GLIpi.SLIk.result_list.length; h++) {
                snprintf(q, sizeof q, "%d",
                        GLIpi.SLIk.result_list.item[h]
                        );
                SEND_STR(q)
            }
            SEND_STR("\n")
        }
    }
    SEND_STR("+-----------+-----------+-----------+------------\n")

    SEND_STR("+-----------------------------------------------------------------------------------+\n")
    SEND_STR("|                                   Sensor                                          |\n")
    SEND_STR("+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n")
    SEND_STR("|    id     | remote_id | peer_id   |   value   |    sec    |   nsec    |   state   |\n")
    SEND_STR("+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n")
    for (int i = 0; i < sensor_fts_list.length; i++) {
        snprintf(q, sizeof q, "|%11d|%11d|%11s|%11.3f|%11ld|%11ld|%11d|\n",
                sensor_fts_list.item[i].id,
                sensor_fts_list.item[i].remote_id,
                sensor_fts_list.item[i].peer.id,
                sensor_fts_list.item[i].value.value,
                sensor_fts_list.item[i].value.tm.tv_sec,
                sensor_fts_list.item[i].value.tm.tv_nsec,
                sensor_fts_list.item[i].value.state
                );
    }
    SEND_STR("+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n")

    acp_sendPeerListInfo(&peer_list, response, &peer_client);

    SEND_STR_L("_")

}

void printHelp(ACPResponse *response) {
    char q[LINE_SIZE];
    SEND_STR("COMMAND LIST\n")
    snprintf(q, sizeof q, "%s\tput process into active mode; process will read configuration\n", ACP_CMD_APP_START);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tput process into standby mode; all running programs will be stopped\n", ACP_CMD_APP_STOP);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tfirst stop and then start process\n", ACP_CMD_APP_RESET);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tterminate process\n", ACP_CMD_APP_EXIT);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget state of process; response: B - process is in active mode, I - process is in standby mode\n", ACP_CMD_APP_PING);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget some variable's values; response will be packed into multiple packets\n", ACP_CMD_APP_PRINT);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget this help; response will be packed into multiple packets\n", ACP_CMD_APP_HELP);
    SEND_STR(q)
    snprintf(q, sizeof q, "%s\tget prog sensor value; program id expected\n", ACP_CMD_GET_FTS);
    SEND_STR_L(q)
}
#undef GLIi
#undef GLIpi
#undef SLIk