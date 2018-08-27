//
// Created by w703710691d on 18-8-24.
//

#ifndef POWERJUDGE_DB_UPDATER_H
#define POWERJUDGE_DB_UPDATER_H

bool init_connet();

void update_solution_status(int cid, int sid, int result, int test);

void close_connet();

#endif //POWERJUDGE_DB_UPDATER_H
