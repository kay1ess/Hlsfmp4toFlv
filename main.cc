/*
 * @Author: kay1ess
 * @Date: 2021-09-28 21:07:36 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-09-28 21:14:55
 */


#include <iostream>
#include <cpr/cpr.h>


int main()
{
    cpr::Response res = cpr::Get(cpr::Url{"http://www.baidu.com"});
    std::cout << "status_code:" << res.status_code << std::endl;
    std::cout << "text:" << res.text << std::endl;
}