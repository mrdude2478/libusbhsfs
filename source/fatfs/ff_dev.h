/*
 * ff_dev.h
 *
 * Copyright (c) 2020, DarkMatterCore <pabloacurielz@gmail.com>.
 * Copyright (c) 2020, XorTroll.
 *
 * This file is part of libusbhsfs (https://github.com/DarkMatterCore/libusbhsfs).
 */

#pragma once

#ifndef __FF_DEV_H__
#define __FF_DEV_H__

#include "../usbhsfs_utils.h"
#include "../usbhsfs_manager.h"

const devoptab_t *ffdev_get_devoptab();

#endif  /* __FF_DEV_H__ */