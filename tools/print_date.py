#!/usr/bin/env python3
import datetime
import time

if __name__ == '__main__':
    utc_offset_sec = time.altzone if time.localtime().tm_isdst else time.timezone
    utc_offset = datetime.timedelta(seconds=-utc_offset_sec)
    print(datetime.datetime.now().replace(microsecond=0).replace(tzinfo=datetime.timezone(offset=utc_offset)).isoformat(' '))