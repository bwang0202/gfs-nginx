# gfs-nginx
make
  177  sudo make install
  178  sudo /usr/local/nginx/sbin/nginx -s stop
  179  sudo /usr/local/nginx/sbin/nginx
  tail -f /usr/local/nginx/logs/error.log