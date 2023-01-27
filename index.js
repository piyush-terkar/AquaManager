const express = require("express");
const path = require("path");
const app = express();
const ip = require("ip");
const ejsMate = require("ejs-mate");
const fs = require("fs");
app.use(require("express-status-monitor")());
app.engine("ejs", ejsMate);
app.set("view engine", "ejs");
app.set("views", path.join(__dirname, "views"));
app.use(express.static(path.join(__dirname, "public")));

app.use(express.urlencoded({ extended: true }));
app.use(express.json());

let CONNECTION = false;
const net = require("net");
const SOCKPORT = 3030;
const PORT = 3000;
const IP = ip.address();

const socket = net.createServer();
socket.listen(SOCKPORT, IP, () => {
  console.log(`Waiting for Controller on ${IP}:${SOCKPORT}`);
});

socket.maxConnections = 1;

socket.on("connection", (sock) => {
  sock.setKeepAlive(true, 1000);
  const unsetConnection = () => {
    console.log("Controller closed connection");
    CONNECTION = false;
  };
  sock.on("close", unsetConnection);
  sock.on("error", unsetConnection);
  sock.on("end", unsetConnection);
  const syncData = JSON.stringify(JSON.parse(fs.readFileSync("./status.json")));
  sock.on("data", (data) => {
    sock.write("sync");
    fs.writeFileSync("./status.json", syncData);
    console.log(data);
  });
  CONNECTION = true;
  sock.write(syncData + "\n");
  fs.watchFile(
    "./status.json",
    { bigint: false, persistent: true, interval: 1000 },
    (curr, prev) => {
      console.log("State change detected, Commiting changes to controller");
      if (CONNECTION) {
        const status = JSON.stringify(
          JSON.parse(fs.readFileSync("./status.json"))
        );
        console.log(status);
        sock.write(status + "\n");
      } else {
        console.log("Changes to be committed");
      }
    }
  );
});

setInterval(() => {
  const status = JSON.parse(fs.readFileSync("./status.json"));
  const settings = JSON.parse(fs.readFileSync("./settings.json"));
  let date = new Date();
  let currtime = date.toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    hour12: false,
  });
  let isChanged = false;
  if (!status.maintainance) {
    if (
      status.filter !=
        (settings.filter.start <= currtime &&
          settings.filter.end >= currtime) &&
      !settings.filter.manual
    ) {
      status.filter =
        settings.filter.start <= currtime && settings.filter.end >= currtime;
      isChanged = true;
    }
    if (
      status.light !=
        (settings.light.start <= currtime && settings.light.end >= currtime) &&
      !settings.light.manual
    ) {
      status.light =
        settings.light.start <= currtime && settings.light.end >= currtime;
      isChanged = true;
      console.log(currtime);
    }
    if (
      status.pump !=
        (settings.pump.start <= currtime && settings.pump.end >= currtime) &&
      !settings.pump.manual
    ) {
      status.pump =
        settings.pump.start <= currtime && settings.pump.end >= currtime;
      isChanged = true;
    }
    if (isChanged) {
      console.log("status changed", status);
      console.log(currtime);
      status.maintainance = status.maintainance;
      fs.writeFileSync("./status.json", JSON.stringify(status));
    }
  }
}, 1000);

// fs.watchFile(
//   "./settings.json",
//   { bigint: false, persistent: true, interval: 1000 },
//   (curr, prev) => {
//     console.log("Settings changed recomputing values");
//     const status = JSON.parse(fs.readFileSync("./status.json"));
//     const settings = JSON.parse(fs.readFileSync("./settings.json"));
//     let date = new Date();
//     let currtime = date.toLocaleTimeString();
//     status.filter =
//       settings.filter.start <= currtime && settings.filter.end >= currtime;
//     status.light =
//       settings.light.start <= currtime && settings.light.end >= currtime;
//     status.pump =
//       settings.pump.start <= currtime && settings.pump.end >= currtime;
//     console.log(status);
//     fs.writeFileSync("./status.json", JSON.stringify(status));
//   }
// );

app.get("/", (req, res) => {
  const config = JSON.parse(fs.readFileSync("./settings.json"));
  const currStatus = JSON.parse(fs.readFileSync("./status.json"));
  res.render("status/home", { status: CONNECTION, config, currStatus });
});

app.get("/edit", (req, res) => {
  const config = JSON.parse(fs.readFileSync("./settings.json"));
  const currStatus = JSON.parse(fs.readFileSync("./status.json"));
  console.log(config);
  res.render("status/edit", { status: CONNECTION, config, currStatus });
});

app.get("/manual", (req, res) => {
  const status = JSON.parse(fs.readFileSync("./status.json"));
  const config = JSON.parse(fs.readFileSync("./settings.json"));
  if (req.query.q) {
    if (req.query.q == "light") {
      status.light = !status.light;
      config.light.manual = true;
    }
    if (req.query.q == "filter") {
      status.filter = !status.filter;
      config.filter.manual = true;
    }
    if (req.query.q == "pump") {
      status.pump = !status.pump;
      config.pump.manual = true;
    }
    if (req.query.q == "auto") {
      config.light.manual = false;
      config.pump.manual = false;
      config.filter.manual = false;
    }
  }
  console.log(config);
  fs.writeFileSync("./status.json", JSON.stringify(status));
  fs.writeFileSync("./settings.json", JSON.stringify(config));
  res.redirect("/");
});

app.post("/edit", (req, res) => {
  const settings = JSON.parse(fs.readFileSync("./settings.json"));
  settings.filter = req.body.filter;
  settings.light = req.body.light;
  settings.pump = req.body.pump;
  settings.filter.manual = settings.filter.manual;
  settings.light.manual = settings.light.manual;
  settings.pump.manual = settings.pump.manual;
  fs.writeFileSync("./settings.json", JSON.stringify(settings));
  res.redirect("/");
});
app.post("/maintainance", async (req, res) => {
  const settings = JSON.parse(fs.readFileSync("./status.json"));
  if (req.body.maintainance) {
    settings.maintainance = true;
    settings.filter = false;
    settings.light = false;
    settings.pump = false;
  } else {
    settings.maintainance = false;
  }
  fs.writeFileSync("./status.json", JSON.stringify(settings));
  res.redirect("/");
});

app.listen(PORT, IP, () => {
  console.log(`listening on ${IP}:${PORT}`);
});
