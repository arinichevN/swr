
#ifndef SWR_MAIN_H
#define SWR_MAIN_H

#include "lib/dbl.h"
#include "lib/util.h"
#include "lib/crc.h"
#include "lib/app.h"
#include "lib/configl.h"
#include "lib/timef.h"
#include "lib/udp.h"
#include "lib/acp/main.h"
#include "lib/acp/app.h"

#define APP_NAME swr
#define APP_NAME_STR TOSTRING(APP_NAME)

#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./"
#endif
#define CONFIG_FILE "" CONF_DIR "config.tsv"

#define WAIT_RESP_TIMEOUT 3

#define FSTR "%.3f"

typedef int Success;
DEC_RLIST(Success)

typedef struct {
    int id;
    FTS *fts;
    SensorFTS *sensor_fts;
    SuccessRList result_list;
    int success_count;
    int used;
} Sensor;

DEC_LIST(Sensor)

typedef struct{
    int id;
    SensorList sensor_list;
    int sensor_rank_size;
} Group;
DEC_LIST(Group)



typedef struct {
    GroupList *list;
     sqlite3 *db;
}GroupData;

typedef struct {
    SensorList *list;
}SensorData;

enum {
    SUCCESS,
    FAILURE
} result;

extern int readSettings() ;

extern void initApp() ;

extern int initData() ;

extern void serverRun(int *state, int init_state) ;

extern void freeData() ;

extern void freeApp() ;

extern void exit_nicely() ;

extern void exit_nicely_e(char *s) ;

#endif 

