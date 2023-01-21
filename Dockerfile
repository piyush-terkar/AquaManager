FROM node:16
WORKDIR /usr/src/app
COPY package*.json ./
RUN npm install
COPY . .
ENV TZ="Asia/Kolkata"
EXPOSE 3000
EXPOSE 3030
CMD ["node", "index.js"]
