/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SECURITY_MANAGER_LOG_H
#define SECURITY_MANAGER_LOG_H

/**
 * @brief Log (>>) a message and arguments
 *
 * @param msg The message to display
 * @param ... arguments
 */
void log_function(const char *msg, ...);

#define LOG(...) log_function(__VA_ARGS__);

/**
 * @brief Display an error with details
 *
 * @param file The file where occurs the error
 * @param line The line where occurs the error
 * @param msg The message to display
 * @param ... arguments
 */
void error_function(const char *file, const int line, const char *msg, ...);

#define ERROR(...) error_function(__FILE__, __LINE__, __VA_ARGS__);

#endif