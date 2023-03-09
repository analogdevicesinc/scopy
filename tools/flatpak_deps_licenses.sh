#!/bin/sh
#set -x

echo "<table>"
for f in $(find ./outdir -name "*.so*" -type f | awk -F/ '{print $NF}' | sed -e 's/\.debug//g' | sort | uniq) ; do
	name=$(basename $f)
	loop=1
	while [ ${loop} -eq 1 ] ; do
#		echo looking for $name
		attempt=1
		while [ ${attempt} -le 5 ] ; do
			wget -o test.url -O test "https://packages.ubuntu.com/search?suite=xenial&arch=amd64&mode=filename&searchon=contents&keywords=${name}"
			if [ $(cat test | wc -l) -ne 0 ] ; then
				break
			fi
			sleep 1
			attempt=$(expr $attempt + 1)
		done
		url=$(sed  '/<\/table>/Q' test | sed -n '/<table>/,$p' | \
			sed '/^[[:space:]]*$/d' | grep -v \<th\> | \
			grep href | sort | uniq -c | head -1 | \
			sed -e 's/^.*href=\"//' -e 's/\".*$//')
#		echo found $url
		if [ $(echo -n $url | wc -c) -eq 0 ] ; then
#			echo trying again
			if [ "${name}" = "$(echo ${name} | sed -e 's/\.[0-9]*$//')" ] ; then
				echo fail with ${name}
				break
			else
				name=$(echo $name | sed -e 's/\.[0-9]*$//')
			fi
		else
#			echo found
			break
		fi
	done

	if [ $(echo -n $url | wc -c) -eq 0 ] ; then
		home="not unstream"
	else
		wget -o /dev/null -O test "https://packages.ubuntu.com/${url}"
		home=$(grep Homepage test | grep \<li\> | sed -e 's:<li>::' -e 's:<\/li>::')
	fi
	if [ $(echo -n $home | wc -c) -eq 0 ] ; then
		home="unknown"
	fi

	echo "<tr><td>$name</td><td>$home</td></tr>"
done

echo "</table>"
