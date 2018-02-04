
#include "main.h"

/*
int checkGroup(const Group *item, const ProgList *list) {
    if (item->matter.mass <= 0) {
        fprintf(stderr, "checkProg(): expected matter_mass > 0 in prog with id = %d\n", item->id);
        return 0;
    }
    if (item->matter.ksh <= 0) {
        fprintf(stderr, "checkProg(): expected matter_ksh > 0 in prog with id = %d\n", item->id);
        return 0;
    }
    if (item->matter.kl < 0) {
        fprintf(stderr, "checkProg(): expected loss_factor >= 0 in prog with id = %d\n", item->id);
        return 0;
    }
    if (item->matter.temperature_pipe.length < 0) {
        fprintf(stderr, "checkProg(): expected temperature_pipe_length >= 0 in prog with id = %d\n", item->id);
        return 0;
    }
    if (getActuatorById(item->cooler.id, list) != NULL) {
        fprintf(stderr, "checkProg(): cooler_id already exists where prog id = %d\n", item->id);
        return 0;
    }
    if (getActuatorById(item->heater.id, list) != NULL) {
        fprintf(stderr, "checkProg(): heater_id already exists where prog id = %d\n", item->id);
        return 0;
    }
    //unique id
    if (getProgById(item->id, list) != NULL) {
        fprintf(stderr, "checkProg(): prog with id = %d is already running\n", item->id);
        return 0;
    }
    return 1;
}
 */

FUN_LIST_INIT(Group)
FUN_LIST_INIT(Sensor)

static int getSensorList_callback(void *data, int argc, char **argv, char **azColName) {
    SensorData *d = data;
    int c = 0;
    for (int i = 0; i < argc; i++) {
        if (DB_COLUMN_IS("sensor_id")) {
            d->list->item[d->list->length].id = atoi(argv[i]);
            c++;
        } else {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): unknown column:%s\n", __func__, DB_COLUMN_NAME);
#endif
            c++;
        }
    }
    d->list->length++;
#define N 1
    if (c != N) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): required %d columns but %d found\n", __func__, N, c);
#endif
        return EXIT_FAILURE;
    }
#undef N
    return EXIT_SUCCESS;
}

static int getSensorList(SensorList *list, int group_id, sqlite3 *db) {
    RESET_LIST(list)
            int n = 0;
    char *qn = "select count(*) FROM group_sensor";
    db_getInt(&n, db, qn);
    if (n <= 0) {
        return 1;
    }
    if (!initSensorList(list, n)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failed to allocate memory\n", __func__);
#endif
        return 0;
    }
    SensorData data = {.list = list};
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "select sensor_id FROM group_sensor where group_id=%d", group_id);
    if (!db_exec(db, q, getSensorList_callback, &data)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): query failed\n", __func__);
#endif
        return 0;
    }
    return 1;
}

static int getGroupList_callback(void *data, int argc, char **argv, char **azColName) {
    GroupData *d = data;
    int c = 0;
    for (int i = 0; i < argc; i++) {
        if (DB_COLUMN_IS("id")) {
            d->list->item[d->list->length].id = atoi(argv[i]);
            c++;
        } else if (DB_COLUMN_IS("sensor_rank_size")) {
            d->list->item[d->list->length].sensor_rank_size = atoi(argv[i]);
            c++;
        } else {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): unknown column:%s\n", __func__, DB_COLUMN_NAME);
#endif
            c++;
        }
    }
#define N 2
    if (c != N) {
        d->list->length++;
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): required %d columns but %d found\n", __func__, N, c);
#endif
        return EXIT_FAILURE;
    }
#undef N
    if (!getSensorList(&d->list->item[d->list->length].sensor_list, d->list->item[d->list->length].id, d->db)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failed to get sensor list where group_id=%d\n", __func__, d->list->item[d->list->length].id);
#endif
    }
    d->list->length++;
    return EXIT_SUCCESS;
}

int getGroupList(GroupList *list, const char *db_path) {
    RESET_LIST(list)
    sqlite3 *db;
    if (!db_open(db_path, &db)) {
        return 0;
    }
    int n = 0;
    char *qn = "select count(*) FROM \"group\"";
    db_getInt(&n, db, qn);
    if (n <= 0) {
        sqlite3_close(db);
        return 1;
    }
    if (!initGroupList(list, n)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failed to allocate memory\n", __func__);
#endif
        sqlite3_close(db);
        return 0;
    }
    GroupData data = {.list = list, .db = db};
    char *q = "select id, sensor_rank_size FROM \"group\"";
    if (!db_exec(db, q, getGroupList_callback, (void*) &data)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): query failed\n", __func__);
#endif
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}
