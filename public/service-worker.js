const CACHE = "SW-cache";

const toCache = ["/", "/edit"];

self.addEventListener("install", function (event) {
  event.waitUntil(
    caches
      .open(CACHE)
      .then((cache) => {
        return cache.addAll(toCache);
      })
      .then(self.skipWaiting())
  );
});

self.addEventListener("fetch", function (event) {
  event.respondWith(
    fetch(event.request).catch(() => {
      const cache = caches.open(CACHE);
      return cache.match(event.request);
    })
  );
});

self.addEventListener("activate", function (event) {
  event.waitUntil(
    caches
      .keys()
      .then((keyList) => {
        return Promise.all(
          keyList.map((key) => {
            if (key !== CACHE) {
              return caches.delete(key);
            }
          })
        );
      })
      .then(() => self.clients.claim())
  );
});
