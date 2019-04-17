mount -o remount,rw /
cp -R /tmp/news /var/www/dashboard
chown -R www-data:www-data /var/www/dashboard/news/*
mount -o remount,ro /
