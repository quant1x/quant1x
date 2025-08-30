# -*- coding: UTF-8 -*-
import os.path

from loguru import logger as __logger

import file, app

__user_home = file.homedir()
__user_home = os.path.expanduser(__user_home)
__logger_path = os.path.join(__user_home, ".quant1x", "logs")

_, filename, _ = app.application()
if filename == 'pythonservice':
    filename = 'quant1x'
log_file = f"{__logger_path}/{filename}.log"
__logger.add(log_file, rotation="00:00", retention="10 days")

logger = __logger

if __name__ == '__main__':
    logger.warning("test")
