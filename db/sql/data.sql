CREATE TABLE "peer" (
    "id" TEXT NOT NULL,
    "port" INTEGER NOT NULL,
    "ip_addr" TEXT NOT NULL
);

CREATE TABLE "sensor_mapping" (
    "sensor_id" INTEGER PRIMARY KEY NOT NULL,
    "peer_id" TEXT NOT NULL,
    "remote_id" INTEGER NOT NULL
);
CREATE TABLE "group_sensor" (
    "group_id" INTEGER NOT NULL,
    "sensor_id" INTEGER NOT NULL
);
CREATE TABLE "group"
(
  "id" INTEGER PRIMARY KEY, -- remote_id for clients
  "sensor_rank_size" INTEGER NOT NULL -- number of last calls to sensors to remember. we will use it to mark the most stable sensor and call it first. if 0 will call sensors ordered by id
);
