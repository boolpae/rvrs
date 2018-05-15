
#include "stas.h"
#include "Notifier.h"
#include "VFCManager.h"
#include "STT2DB.h"

#include <sys/inotify.h>
#include <unistd.h>

/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>

#include <log4cpp/Category.hh>

#include <chrono>
#include <cstring>
#include <fstream>

Notifier* Notifier::m_instance = nullptr;



Notifier::Notifier(VFCManager *vfcm, STT2DB *stt2db)
: m_vfcm(vfcm), m_stt2db(stt2db), m_LiveFlag(true)
{
    
}

Notifier::~Notifier()
{
    if (m_thrdNoti.joinable()) m_thrdNoti.join();
}

Notifier* Notifier::instance(VFCManager *vfcm, STT2DB *stt2db)
{
    if (!m_instance) {
        m_instance = new Notifier(vfcm, stt2db);
    }
    
    return m_instance;
}

int Notifier::startWork()
{
    log4cpp::Category *logger = config->getLogger();
    
	if (!config->isSet("notify.input_path")) {
		logger->fatal("Not set inotify.input_path");
		return 1;
	}

	std::shared_ptr<std::string> path = std::make_shared<std::string>(config->getConfig("notify.input_path"));
	logger->info("Initialize monitoring module to watch %s", path->c_str());
	if (!itfact::common::checkPath(*path.get(), true)) {
		logger->error("Cannot create directory '%s' with error %s", path->c_str(), std::strerror(errno));
		return 1;
	}
#if 0
	int inotify = inotify_init();
	if (inotify < 0) {
		int rc = errno;
		logger->error("Cannot initialization iNotify '%s'", std::strerror(rc));
		return rc;
	}

	int wd = inotify_add_watch(inotify, path->c_str(), IN_CLOSE_WRITE);
	if (wd < 0) {
		int rc = errno;
		logger->error("Cannot watch '%s' with error %s", path->c_str(), std::strerror(rc));
		return rc;
	}
#endif
    m_thrdNoti = std::thread(Notifier::thrdFunc, this);
    
    return 0;
}


#define BUF_LEN (10 * (sizeof(struct inotify_event) + FILENAME_MAX + 1))

void Notifier::thrdFunc(Notifier *noti)
{
    struct timeval tv;
    fd_set rfds;
    int selVal;
	char buf[BUF_LEN] __attribute__ ((aligned(8)));
    log4cpp::Category *logger = config->getLogger();
	std::shared_ptr<std::string> path = std::make_shared<std::string>(config->getConfig("notify.input_path"));
    
	int inotify = inotify_init();
	if (inotify < 0) {
		int rc = errno;
		logger->error("Cannot initialization iNotify '%s'", std::strerror(rc));
		return;
	}

	int wd = inotify_add_watch(inotify, path->c_str(), IN_CLOSE_WRITE);
	if (wd < 0) {
		int rc = errno;
		logger->error("Cannot watch '%s' with error %s", path->c_str(), std::strerror(rc));
		return;
	}

	std::string watch_ext = config->getConfig("notify.watch", "txt");

    while(noti->m_LiveFlag) {
		tv.tv_sec = 0;	// for debug
		tv.tv_usec = 500000;
		FD_ZERO(&rfds);
		FD_SET(inotify, &rfds);

		selVal = select(inotify+1, &rfds, NULL, NULL, &tv);

        if (selVal > 0) {
            ssize_t numRead = read(inotify, buf, BUF_LEN);
            if (numRead <= 0) {
                int rc = errno;
                if (rc == EINTR)
                    continue;

                logger->warn("Error occurred: (%d), %s", rc, std::strerror(rc));
                break;
            }

            struct inotify_event *event = NULL;
            for (char *p = buf; p < buf + numRead; p += sizeof(struct inotify_event) + event->len) {
                event = (struct inotify_event *) p;
                if (!(event->mask & IN_ISDIR)) {
                    // Call Job
                    std::shared_ptr<std::string> filename = std::make_shared<std::string>(event->name, event->len);
                    std::string file_ext = filename->substr(filename->rfind(".") + 1);

                    logger->debug("Noti file %s (Watch: '%s', ext: '%s')", filename->c_str(), watch_ext.c_str(), file_ext.c_str());

                    if (filename->at(0) != '.' && file_ext.find(watch_ext) == 0 &&
                        (file_ext.size() == watch_ext.size() || file_ext.at(watch_ext.size()) == '\0' )) {
                        try {
                            // option값에 따라 동작이 바뀌어야 한다.
                            // pushItem() 시 protocol 추가해야한다. - FILE, MOUNT, HTTP, HTTPS, FTP, FTPS, SFTP, SCP, SSH
                            if (config->getConfig("notify.index_type").compare("filename") == 0) {

                                // download path, uri, filename, call_id에 대한 좀 더 명확한 정의가 필요하다.
                                if (noti->m_stt2db->searchTaskInfo(*path.get(), *filename.get(), std::string(""))) continue;
                                noti->m_stt2db->insertTaskInfo(*path.get(), *filename.get(), std::string(""));

                                noti->m_vfcm->pushItem(*path.get()+"/"+*filename.get());
                                logger->debug("Line (%s)", std::string(*path.get()+"/"+*filename.get()).c_str());
                            }
                            else {
                                std::string pathfile = *path.get()+"/"+*filename.get();
                                std::ifstream index_file(pathfile);
                                if (index_file.is_open()) {
                                    for (std::string line; std::getline(index_file, line); ) {
                                        if (line.empty() || line.size() < 5)
                                            continue;

                                        if (noti->m_stt2db->searchTaskInfo(*path.get(), *filename.get(), std::string(""))) continue;
                                        noti->m_stt2db->insertTaskInfo(*path.get(), *filename.get(), std::string(""));

                                        noti->m_vfcm->pushItem(line);
                                        logger->debug("Line (%s)", line.c_str());
                                    }
                                    index_file.close();
                                }
                            }

                            bool delete_on_list = const_cast<itfact::common::Configuration *>(config)->getConfig<bool>("notify.delete_on_list");
                            if (delete_on_list)
                                std::remove(filename->c_str());

                        } catch (std::exception &e) {
                            logger->warn("%s: %s", e.what(), filename->c_str());
                        }
                    } else {
                        logger->debug("Ignore %s (Watch: '%s', ext: '%s')", filename->c_str(),
                                        watch_ext.c_str(), file_ext.c_str());
                    }
                }
            }
        }
    }

	inotify_rm_watch(inotify, wd);
}