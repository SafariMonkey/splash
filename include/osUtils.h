/*
 * Copyright (C) 2015 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @osUtils.h
 * Some system utilities
 */

#ifndef SPLASH_OSUTILS_H
#define SPLASH_OSUTILS_H

#include <string>
#include <vector>
#include <unistd.h>
#if HAVE_SHMDATA
    #include <shmdata/abstract-logger.hpp>
#endif
#include <pwd.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "./log.h"

namespace Splash
{
    namespace Utils
    {
        /*****/
#if HAVE_LINUX
        inline int getThreadId()
        {
            pid_t threadId;
            threadId = syscall(SYS_gettid);
            return threadId;
        }
#endif

        /*****/
        inline int getCoreCount()
        {
            auto ncores = sysconf(_SC_NPROCESSORS_CONF);
            return ncores;
        }

        /*****/
        inline bool setAffinity(const std::vector<int>& cores)
        {
#if HAVE_LINUX
            auto ncores = getCoreCount();
            for (auto& core : cores)
                if (core >= ncores)
                    return false;

            cpu_set_t set;
            CPU_ZERO(&set);
            for (auto& core : cores)
                CPU_SET(core, &set);

            if (sched_setaffinity(getThreadId(), sizeof(set), &set) != 0)
                return false;

            return true;
#else
            return false;
#endif
        }

        /*****/
        inline bool setRealTime()
        {
#if HAVE_LINUX
            sched_param params;
            params.sched_priority = 99;
            if (sched_setscheduler(getThreadId(), SCHED_RR, &params) != 0)
                return false;

            return true;
#else
            return false;
#endif
        }

        /*****/
        inline bool isDir(const std::string& filepath)
        {
            struct stat pathStat;
            if (lstat(filepath.c_str(), &pathStat) == -1)
                return false;
            return S_ISDIR(pathStat.st_mode);
        }

        /*****/
        inline std::string cleanPath(const std::string& filepath)
        {
            std::vector<std::string> links;

            auto remain = filepath;
            while (remain.size() != 0)
            {
                auto nextSlashPos = remain.find("/");
                if (nextSlashPos == 0)
                {
                    remain = remain.substr(1, std::string::npos);
                    continue;
                }
                
                auto link = remain.substr(0, nextSlashPos);
                links.push_back(link);

                if (nextSlashPos == std::string::npos)
                    remain.clear();
                else
                    remain = remain.substr(nextSlashPos + 1, std::string::npos);
            }

            for (int i = 0; i < links.size();)
            {
                if (links[i] == "..")
                {
                    links.erase(links.begin() + i);
                    if (i > 0)
                        links.erase(links.begin() + i - 1);
                    i -= 1;
                    continue;
                }

                if (links[i] == ".")
                {
                    links.erase(links.begin() + i);
                    continue;
                }

                ++i;
            }

            auto path = std::string("");
            for (auto& link : links)
            {
                path += "/";
                path += link;
            }

            if (path.size() == 0)
                path = "/";

            if (isDir(path) && path[path.size() - 1] != '/')
                path += "/";

            return path;
        }

        /*****/
        inline std::string getHomePath()
        {
            if (getenv("HOME"))
                return std::string(getenv("HOME"));

            struct passwd* pw = getpwuid(getuid());
            return std::string(pw->pw_dir);
        }

        /*****/
        inline std::string getPathFromFilePath(const std::string& filepath, const std::string& configPath = "")
        {
            auto path = filepath;

            bool isRelative = path.find(".") == 0 ? true : false;
            bool isAbsolute = path.find("/") == 0 ? true : false;
            auto fullPath = std::string("");

            if (!isRelative && !isAbsolute)
            {
                isRelative = true;
                path = "./" + filepath;
            }

            size_t slashPos = path.rfind("/");

            if (isAbsolute)
                fullPath = path.substr(0, slashPos) + "/";
            else if (isRelative)
            {
                if (configPath.size() == 0)
                {
                    char workingPathChar[256];
                    auto workingPath = std::string(getcwd(workingPathChar, 255));
                    if (path.find("/") == 1)
                        fullPath = workingPath + path.substr(1, slashPos) + "/";
                    else if (path.find("/") == 2)
                        fullPath = workingPath + "/" + path.substr(0, slashPos) + "/";
                }
                else
                {
                    fullPath = configPath + "/" + path.substr(0, slashPos);
                }
            }

            return cleanPath(fullPath);
        }

        /*****/
        inline std::string getPathFromExecutablePath(const std::string& filepath)
        {
            auto path = filepath;

            bool isRelative = path.find(".") == 0 ? true : false;
            bool isAbsolute = path.find("/") == 0 ? true : false;
            auto fullPath = std::string("");

            size_t slashPos = path.rfind("/");

            if (isAbsolute)
            {
                fullPath = path.substr(0, slashPos) + "/";
                fullPath = cleanPath(fullPath);
            }
            else if (isRelative)
            {
                char workingPathChar[256];
                auto workingPath = std::string(getcwd(workingPathChar, 255));
                if (path.find("/") == 1)
                    fullPath = workingPath + path.substr(1, slashPos) + "/";
                else if (path.find("/") == 2)
                    fullPath = workingPath + "/" + path.substr(0, slashPos) + "/";
                fullPath = cleanPath(fullPath);
            }

            return fullPath;
        }

        /*****/
        inline std::string getFilenameFromFilePath(const std::string& filepath)
        {
            size_t slashPos = filepath.rfind("/");
            auto filename = std::string("");
            if (slashPos == std::string::npos)
                filename = filepath;
            else
                filename = filepath.substr(slashPos + 1);
            return filename;
        }
    
#if HAVE_SHMDATA
        /*****/
        // A shmdata logger dedicated to splash
        class ConsoleLogger: public shmdata::AbstractLogger
        {
            private:
                void on_error(std::string &&str) final
                {
                    Log::get() << Log::ERROR << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
                void on_critical(std::string &&str) final
                {
                    Log::get() << Log::ERROR << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
                void on_warning(std::string &&str) final
                {
                    Log::get() << Log::WARNING << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
                void on_message(std::string &&str) final
                {
                    Log::get() << Log::MESSAGE << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
                void on_info(std::string &&str) final
                {
                    Log::get() << Log::MESSAGE << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
                void on_debug(std::string &&str) final
                {
                    Log::get() << Log::DEBUGGING << "Shmdata::ConsoleLogger - " << str << Log::endl;
                }
        };
#endif
    } // end of namespace
} // end of namespace

#endif
