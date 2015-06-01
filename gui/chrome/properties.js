function startDownload(event)
{
    var p, c, e;

    p = [];
    c = {};

    e = document.getElementById("output");
    if (e.value !== "")
        p.push(e.value);

    e = document.getElementById("filename");
    if (e.value !== "")
        p.push(e.value);

    if (p.length > 0)
        c.output = p.join("/");

    e = document.getElementById("num_connections");
    if (e.value > 0)
        c.num_connections = e.valueAsNumber;

    e = document.getElementById("max_speed");
    if (e.value > 0)
        c.max_speed = e.valueAsNumber;

    e = document.getElementById("user_agent");
    if (e.value !== "")
        c.user_agent = e.value;

    e = document.getElementById("url");
    chrome.runtime.getBackgroundPage(function(background)
    {
        background.startDownload(c, [e.value]);
        exitWindow();
    });
}

function loadOptions()
{
    var u, c;

    u = location.search.substring(2);

    c = Object.create(conf_t);
    conf_init(c);

    document.getElementById("url").value = u;

    if ("output" in c)
        document.getElementById("output").value = c.output;

    if ("num_connections" in c)
        document.getElementById("num_connections").value = c.num_connections;

    if ("max_speed" in c)
        document.getElementById("max_speed").value = c.max_speed;

    if ("user_agent" in c)
        document.getElementById("user_agent").value = c.user_agent;
}

function fixValue(event)
{
    event.target.value = event.target.value.trim();
}

function exitWindow(event)
{
    window.close();
}

document.getElementById("main").addEventListener("blur", fixValue, true);
document.getElementById("start").addEventListener("click", startDownload);
document.getElementById("exit").addEventListener("click", exitWindow);
document.addEventListener("DOMContentLoaded", loadOptions);
