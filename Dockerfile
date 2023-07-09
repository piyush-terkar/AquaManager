FROM node:16
WORKDIR /usr/src/app
COPY package*.json ./
COPY ./certificates/apache-selfsigned.crt /etc/ssl/certs/app.crt
COPY ./certificates/apache-selfsigned.key /etc/ssl/private/app.key
RUN npm install
COPY . .
ENV TZ="Asia/Kolkata"
EXPOSE 3000
EXPOSE 3030
CMD ["node", "index.js"]
