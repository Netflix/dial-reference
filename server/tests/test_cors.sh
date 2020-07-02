#!/bin/bash

if [ $# -eq 0 ]; then
    echo "usage: `basename $0`: <DIAL server ip address> <port> "
    exit 1
fi

ip_address=$1
port=$2

#Testing all the positive cases
origins="https://www.netflix.com https://netflix.com https://port.netflix.com:123 https://www.netflix.com:80 https://www.netflix.com:123 proto://netflix.com proto://netflix proto://netflix.com:123"
for origin in $origins; do
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/Netflix | grep -q "403" && echo "failed[p0]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4" -X POST  http://$ip_address:$port/apps/Netflix || echo "failed[p1]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix | grep -q "403" &&  echo "failed[p2]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET http://$ip_address:$port/apps/Netflix | grep -q "403" && echo "failed[p3]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run | grep -q "403" && echo "failed[p4]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X DELETE  http://$ip_address:$port/apps/Netflix/run | grep -q "403" && echo "failed[p5]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/Netflix/run/hide | grep -q "403" && echo "failed[p6]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run/hide | grep -q "403" && echo "failed[p7]: $origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/dial_data | grep -q "403" && echo "failed[p8]: $origin should be accepted"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/Netflix/dial_data | grep -q "403" && echo "failed[p9]: $origin should be accepted"
fi
done

origins="https://www.youtube.com https://music.youtube.com https://youtube.com https://port.youtube.com:123 https://www.youtube.com:80 https://www.youtube.com:123 package:com.google.android.youtube package:com.google.ios.youtube proto:g proto:com.google"
for origin in $origins; do
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[p10]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4" -X POST  http://$ip_address:$port/apps/YouTube || echo "failed[p11]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[p12]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[p13]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run | grep -q "403" && echo "failed[p14]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X DELETE  http://$ip_address:$port/apps/YouTube/run | grep -q "403" && echo "failed[p15]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/run/hide | grep -q "403" && echo "failed[p16]: $origin should be accepted"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run/hide | grep -q "403" && echo "failed[p17]: $origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" && echo "failed[p18]: $origin should be accepted"
    curl --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" && echo "failed[p19]: $origin should be accepted"
fi
done

#Testing all the negative cases
origins="http://www.netflix-a.com http://www.netflix.com4 http://a-netflix.com http://www4.netflix.com https://port.netflix.com:1234 http://1.netflix.com https://www4.netflix.com https://ww.netflix-a.com https://www.netflix.com4 https://a-netflix.com http://netflix.com http://www.attack.com https://www.attack.com file://www.attack.com ftp://this.is.not.fine package: package:com.netflix.null proto:// proto:n proto:/n proto"
for origin in $origins; do
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/Netflix | grep -q "403" || echo "failed[n0]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4" -X POST  http://$ip_address:$port/apps/Netflix && echo "failed[n1]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix | grep -q "403" ||  echo "failed[n2]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET http://$ip_address:$port/apps/Netflix | grep -q "403" || echo "failed[n3]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run | grep -q "403" || echo "failed[n4]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X DELETE  http://$ip_address:$port/apps/Netflix/run | grep -q "403" || echo "failed[n5]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET http://$ip_address:$port/apps/Netflix/run | grep -q "403" || echo "failed[n6]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run/hide | grep -q "403" || echo "failed[n7]: $origin should be rejected"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/Netflix/dial_data | grep -q "403" || echo "failed[n8]: $origin should be rejected"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/Netflix/dial_data | grep -q "403" || echo "failed[n9]: $origin should be rejected"
fi
done

origins="http://www.youtube-a.com http://www.youtube.com4 https://.youtube.com http://a-youtube.com https://ww.youtube-a.com http://www4.youtube.com https://port.youtube.com:1234 http://1.youtube.com https://www.youtube.com4 https://a-youtube.com http://youtube.com http://www.attack.com https://www.attack.com file://www.attack.com ftp://this.is.not.fine packagecom.google.android.youtube package:com.google.android.utube packagea package: pack:com.google.android protoa proto:"
for origin in $origins; do
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET http://$ip_address:$port/apps/YouTube | grep -q "403" ||  echo "failed[n10]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4" -X POST  http://$ip_address:$port/apps/YouTube && echo "failed[n11]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube | grep -q "403" || echo "failed[n12]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube | grep -q "403" || echo "failed[n13]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run | grep -q "403" || echo "failed[n14]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X DELETE  http://$ip_address:$port/apps/YouTube/run | grep -q "403" || echo "failed[n15]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/run | grep -q "403" || echo "failed[n16]: $origin should be rejected"
curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run/hide | grep -q "403" || echo "failed[n17]: $origin should be rejected"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" || echo "failed[n18]: $origin should be rejected"
    curl --output /dev/null --fail --silent --header "Origin:$origin" -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" || echo "failed[n19]: $origin should be rejected"
fi
done

#Finally test with no header
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[o0]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent --data "v=QH2-TGUlwu4" -X POST  http://$ip_address:$port/apps/YouTube || echo "failed[o1]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[o2]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube | grep -q "403" && echo "failed[o3]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run | grep -q "403" && echo "failed[o4]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X DELETE  http://$ip_address:$port/apps/YouTube/run | grep -q "403" && echo "failed[o5]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/run/hide | grep -q "403" && echo "failed[o6]: request without an Origin should be accepted"
curl --output /dev/null --fail --silent -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run/hide | grep -q "403" && echo "failed[o7]: request without an Origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address with no origin"
    curl --output /dev/null --fail --silent -I -w "%{http_code}" -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" && echo "failed[o8]: request without an Origin should be accepted"
    curl --output /dev/null --fail --silent -I -w "%{http_code}" -X GET  http://$ip_address:$port/apps/YouTube/dial_data | grep -q "403" && echo "failed[o9]: request without an Origin should be accepted"
fi

echo "Done."
