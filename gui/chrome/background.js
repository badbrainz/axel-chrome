var downloads = {};

function startDownload(conf, urls)
{
    var a, c, i;

    c = Object.create(conf_t);
    conf_init(c);
    for (i in conf)
    {
        c[i] = conf[i];
    }

    a = axel_new(c, urls);
    if (a.ready != -1)
    {
        axel_start(a);
    }

    downloads[a.id] = a;
}

function cancelDownload(id)
{
    if (id in downloads)
    {
        axel_close(downloads[id]);
    }
}

function retryDownload(id)
{
    if (id in downloads)
    {
        if (downloads[id].ready === -1)
        {
            axel_start(downloads[id]);
        }
    }
}

function removeDownload(id)
{
    if (id in downloads)
    {
        if (downloads[id].ready !== 0)
        {
            delete downloads[id];
        }
    }
}

function getAllDownloads()
{
    var i, l;

    l = [];
    for (i in downloads)
    {
        l.push(downloads[i]);
    }

    return l;
}

function openPage(fn)
{
    var u;

    u = chrome.runtime.getURL(fn + ".html");

    chrome.tabs.query({url: u}, function(tabs)
    {
        if (tabs.length > 0)
        {
            chrome.windows.update(tabs[0].windowId, {focused: true}, function()
            {
                chrome.tabs.update(tabs[0].id, {active: true});
            });
        }
        else
        {
            chrome.tabs.create({url: u,active: true});
        }

    });
}

chrome.contextMenus.create({
    id: "save",
    title: "Save Link",
    contexts: ["link", "selection"],
    documentUrlPatterns: ["*://*/*", "chrome://*/"],
    onclick: function(info, tab)
    {
        var u;

        u = info.selectionText || info.linkUrl;

        startDownload({}, [u]);
    }
});

chrome.contextMenus.create({
    id: "properties",
    title: "Save Link As...",
    contexts: ["link", "selection"],
    documentUrlPatterns: ["*://*/*", "chrome://*/"],
    onclick: function(info, tab)
    {
        var u;

        u = info.selectionText || info.linkUrl;

        chrome.windows.create({
            width: 630,
            height: 300,
            type: "popup",
            focused: true,
            url: chrome.runtime.getURL("properties.html") + "?=" + u
        });
    }
});

chrome.contextMenus.create({
    id: "separator",
    type: "separator",
    contexts: ["page", "frame", "link", "selection"],
    documentUrlPatterns: ["*://*/*", "chrome://*/"]
});

chrome.contextMenus.create({
    id: "downloads",
    title: "Downloads",
    contexts: ["page", "frame", "link", "selection"],
    documentUrlPatterns: ["*://*/*", "chrome://*/"],
    onclick: function(info, tab)
    {
        openPage("downloads");
    }
});

chrome.contextMenus.create({
    id: "options",
    title: "Options",
    contexts: ["page", "frame", "link", "selection"],
    documentUrlPatterns: ["*://*/*", "chrome://*/"],
    onclick: function(info, tab)
    {
        openPage("options");
    }
});

chrome.runtime.onMessageExternal.addListener(function(message, sender, response)
{
	if ("save" in message)
	{
		startDownload(message.save.conf, message.save.urls);
	}
});
