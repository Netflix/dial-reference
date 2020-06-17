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
curl --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4"  http://$ip_address:$port/apps/Netflix || echo "failed: $origin should be accepted"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix || echo "failed: $origin should be accepted"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run || echo "failed: $origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix/dial_data || echo "failed: $origin should be accepted"
fi
done

origins="https://www.youtube.com https://music.youtube.com https://youtube.com https://port.youtube.com:123 https://www.youtube.com:80 https://www.youtube.com:123 package:com.google.android.youtube package:com.google.ios.youtube proto:g proto:com.google"
for origin in $origins; do
curl --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4"  http://$ip_address:$port/apps/YouTube || echo "failed: $origin should be accepted"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube || echo "failed: $origin should be accepted"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run || echo "failed: $origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data || echo "failed: $origin should be accepted"
fi
done

#Testing all the negative cases
origins="http://www.netflix-a.com http://www.netflix.com4 http://a-netflix.com http://www4.netflix.com https://port.netflix.com:1234 http://1.netflix.com https://www4.netflix.com https://ww.netflix-a.com https://www.netflix.com4 https://a-netflix.com http://netflix.com http://www.attack.com https://www.attack.com file://www.attack.com ftp://this.is.not.fine package: package:com.netflix.null proto:// proto:n proto:/n proto"
for origin in $origins; do
curl --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4"  http://$ip_address:$port/apps/Netflix && echo "failed: $origin should be rejected"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix && echo "failed: $origin should be rejected"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix/run && echo "failed: $origin should be rejected"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/Netflix/dial_data && echo "failed: $origin should be rejected"
fi
done

origins="http://www.youtube-a.com http://www.youtube.com4 https://.youtube.com http://a-youtube.com https://ww.youtube-a.com http://www4.youtube.com https://port.youtube.com:1234 http://1.youtube.com https://www.youtube.com4 https://a-youtube.com http://youtube.com http://www.attack.com https://www.attack.com file://www.attack.com ftp://this.is.not.fine packagecom.google.android.youtube package:com.google.android.utube packagea package: pack:com.google.android protoa proto:"
for origin in $origins; do
curl --fail --silent --header "Origin:$origin" --data "v=QH2-TGUlwu4"  http://$ip_address:$port/apps/YouTube && echo "failed: $origin should be rejected"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube && echo "failed: $origin should be rejected"
curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube/run && echo "failed: $origin should be rejected"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address from origin $origin"
    curl --fail --silent --header "Origin:$origin" -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data && echo "failed: $origin should be rejected"
fi
done

#Finally test with no header
curl --fail --silent --data "v=QH2-TGUlwu4"  http://$ip_address:$port/apps/YouTube || echo "failed: request without an Origin should be accepted"
curl --fail --silent -X OPTIONS  http://$ip_address:$port/apps/YouTube || echo "failed: request without an Origin should be accepted"
curl --fail --silent -X OPTIONS  http://$ip_address:$port/apps/YouTube/run || echo "failed: request without an Origin should be accepted"
if [ $ip_address == "localhost" ];
then
    echo "testing dial_data OPTIONS on $ip_address with no origin"
    curl --fail --silent -X OPTIONS  http://$ip_address:$port/apps/YouTube/dial_data || echo "failed: request without an Origin should be accepted"
fi

echo "Done."
