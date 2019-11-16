#ifndef __T15Q5_SERVER_COMM__
#define __T15Q5_SERVER_COMM__

/*
 * Sends a request to the configured server ato check
 * if the fob_id and fob_code are authorized.
 *
 * @returns 0 if successful, 403 if forbitten
 */
int server_comm_make_request(int fob_id, int fob_code);
#endif

