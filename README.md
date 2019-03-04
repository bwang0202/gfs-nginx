# gfs-nginx
sudo vi /usr/local/nginx/conf/nginx.conf

        location /write/ {
            root   html;
            index  index.html index.htm;
            gfs;
            csid cksr1;
            chunksize 64k;
            #max_batch 5;
            root_dir /tmp/;
        }

        location /gfs_put/ {
            proxy_pass http://127.0.0.1:8888;
        }

  370  ./configure --add-module=/home/bojun/nginx/gfs-nginx
  371  make
  372  sudo /usr/local/nginx/sbin/nginx -s stop
  373  sudo make install
  374  sudo /usr/local/nginx/sbin/nginx
  375  tail -f /usr/local/nginx/logs/error.log

  8192 bytes per write request, configurable by changing read_client_Request file threshhold.

