#pragma once

//see ofUtils.cpp->defaultDataPath()

#if defined TARGET_OSX
//default "../../../data/"
const string __data_path__ = string("../../../../../data/");

#elif defined TARGET_ANDROID
const string __data_path__ = string("sdcard/");

#elif defined(TARGET_LINUX) || defined(TARGET_WIN32)
//default "data/"
const string __data_path__ = string(ofFilePath::join(ofFilePath::getCurrentExeDir(), "../../data/"));

#else
//default "data/"
const string __data_path__ = string("../../data/");

#endif

