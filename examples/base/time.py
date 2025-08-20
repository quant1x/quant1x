def _format_time(time_stamp):
    """
    format time from reversed_bytes0
    by using method from https://github.com/rainx/pytdx/issues/187
    """
    time = time_stamp[:-6] + ':'
    if int(time_stamp[-6:-4]) < 60:
        time += '%s:' % time_stamp[-6:-4]
        time += '%06.3f' % (
                int(time_stamp[-4:]) * 60 / 10000.0
        )
    else:
        time += '%02d:' % (
                int(time_stamp[-6:]) * 60 / 1000000
        )
        time += '%06.3f' % (
                (int(time_stamp[-6:]) * 60 % 1000000) * 60 / 1000000.0
        )
    return time

print(_format_time('9762624'))