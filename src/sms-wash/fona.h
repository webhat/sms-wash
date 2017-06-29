
/* Copyright (c) 2013 Daniël W. Crompton <daniel@specialbrands.net>,
 *     Special Brands <info@specialbrands.net>
 *     */

#ifndef FONA_H
#define FONA_H

void fona_setup();
void fona_send(char*);
bool fona_post_http(char*);
void fona_enable_gprs();
void fona_read_sms(bool (*callback)(char * replybuffer));

#endif /*FONA_H*/
