#ifndef DESKTOPSERVICE_H
#define DESKTOPSERVICE_H
#include <string>
#include "service.h"

namespace org_restfulipc
{

    class DesktopService : public Service
    {
    public:
        DesktopService();
        virtual ~DesktopService();
    private:
        void init(std::string applicationsDir, std::string subDir);

        std::list<std::string> applicationsDirs;
    };

}
#endif /* DESKTOPSERVICE_H */

