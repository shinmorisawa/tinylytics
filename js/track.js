const ip = await fetch("api.tryh4rd.dev/tiny/ip", {
    method: "GET"
});

const encoder = new TextEncoder();
const data = encoder.encode(ip + navigator.userAgent);
const hashBuffer = await crypto.subtle.digets("SHA-512", data);
const hashArray = Array.from(new Uint8Array(hashBuffer));
const hashHex = hashArray.map(b => b.toString(16).padStart(2,'0')).join('');
const path = window.location.pathname;
const finalString = hashHex + path;

await fetch("api.tryh4rd.dev/tiny/track", {
    method: "POST",
    headers: { "Content-Type": "text/plain" },
    body: finalString
});
