
#include "stas.h"
#include "Notifier.h"
#include "VFCManager.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <log4cpp/Category.hh>

#include <chrono>
#include <cstring>

Notifier* Notifier::m_instance = nullptr;



Notifier::Notifier(VFCManager *vfcm)
: m_vfcm(vfcm), m_LiveFlag(true)
{
    
}

Notifier::~Notifier()
{
    if (m_thrdNot.joinable()) m_thrdNot.join();
}

Notifier* Notifier::instance(VFCManager *vfcm)
{
    if (!m_instance) {
        m_instance = new Notifier(vfcm);
    }
    
    return m_instance;
}

int Notifier::startWork()
{
    log4cpp::Category *logger = config->getLogger();
    
	if (!config->isSet("inotify.input_path")) {
		logger->fatal("Not set inotify.input_path");
		return 1;
	}

	std::shared_ptr<std::string> path = std::make_shared<std::string>(config->getConfig("inotify.input_path"));
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
    m_thrdNot = std::thread(Notifier::thrdFunc, this);
    
    return 0;
}


#define BUF_LEN (10 * (sizeof(struct inotify_event) + FILENAME_MAX + 1))

void Notifier::thrdFunc(Notifier *noti)
{
	char buf[BUF_LEN] __attribute__ ((aligned(8)));
    log4cpp::Category *logger = config->getLogger();
	std::shared_ptr<std::string> path = std::make_shared<std::string>(config->getConfig("inotify.input_path"));
    
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

	std::string watch_ext = config->getConfig("inotify.watch", "txt");

    while(noti->m_LiveFlag) {
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
				if (filename->at(0) != '.' && file_ext.find(watch_ext) == 0 &&
					(file_ext.size() == watch_ext.size() || file_ext.at(watch_ext.size()) == '\0' )) {
					try {
                        
                        // option값에 따라 동작이 바뀌어야 한다.
#if 0
						std::thread job(VRInotify::runJob, path, filename, &config);
						job.detach();
#endif
                        noti->m_vfcm->pushItem(*path.get());
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

	inotify_rm_watch(inotify, wd);
}