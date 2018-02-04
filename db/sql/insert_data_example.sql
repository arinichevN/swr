delete from sensor_mapping;
INSERT INTO sensor_mapping (sensor_id,peer_id,remote_id) VALUES (1,'obj_1',1);
INSERT INTO sensor_mapping (sensor_id,peer_id,remote_id) VALUES (2,'obj_1',2);
INSERT INTO sensor_mapping (sensor_id,peer_id,remote_id) VALUES (3,'gwu18_1',1);
INSERT INTO sensor_mapping (sensor_id,peer_id,remote_id) VALUES (4,'gwu18_1',2);

delete from "group";
INSERT INTO "group"(id,sensor_rank_size) VALUES (1,10);

delete from "group_sensor";
INSERT INTO "group_sensor"(group_id, sensor_id) VALUES (1,1);
INSERT INTO "group_sensor"(group_id, sensor_id) VALUES (1,2);
INSERT INTO "group_sensor"(group_id, sensor_id) VALUES (1,3);
INSERT INTO "group_sensor"(group_id, sensor_id) VALUES (1,4);



