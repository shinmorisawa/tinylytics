async function main() {
    const ip = await fetch("https://api.tryh4rd.dev/tiny/ip", {
        method: "GET"
    });

    const encoder = new TextEncoder();
    const data = encoder.encode(ip + navigator.userAgent);
    const hashBuffer = await crypto.subtle.digest("SHA-512", data);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    const hashHex = hashArray.map(b => b.toString(16).padStart(2,'0')).join('');
    var path = window.location.pathname;
    path = path.normalize('NFC');
    const finalString = hashHex + path;

    console.log(path);

    await fetch("https://api.tryh4rd.dev/tiny/track", {
        method: "POST",
        headers: { "Content-Type": "text/plain" },
        body: finalString
    });
}

main();
