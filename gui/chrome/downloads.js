//For displaying download info
function Display(download)
{
    this.id = download.id;
    this.element = createElement("download-" + download.id);
}

Display.prototype.update = function(download)
{
    var c;

    c = "";
    if (download.ready === 0)
    {
        c = "active";
    }
    else if (download.ready === 1)
    {
        c = "done";
    }
    else if (download.ready === -1)
    {
        c = download.size === 0 ? "error" : "incomplete";
    }

    this.element.className = "download " + c;
    this.element.querySelector(".size").innerText = "(" + sizeHuman(download.size) + ")";
    this.element.querySelector(".file").innerText = download.filename;
    this.element.querySelector(".url").innerText = download.url.text;
    this.element.querySelector(".url").href = download.url.text;
    this.element.querySelector(".message").innerHTML = download.message.trim().replace(/\s/g, '&nbsp;');
}

//Holds Display instances
function DownloadManager()
{
    this.collection = {};
    this.node = document.getElementById("downloads");
}

//Remove all Display instances
DownloadManager.prototype.clear = function()
{
    var k;

    for (k in this.collection)
    {
        delete this.collection[k];
    }

    this.node.innerHTML = "";
};

//Update Display instance
DownloadManager.prototype.update = function(download)
{
    var i;

    i = this.collection[download.id];
    if (i == null)
    {
        i = new Display(download);
        this.collection[download.id] = i;
        this.node.insertBefore(i.element, this.node.firstElementChild);
    }

    i.update(download);
};

//Compare list of downloads to current list
DownloadManager.prototype.needsUpdate = function(list)
{
    var l, k;

    l = 0;
    for (k in this.collection)
        l++;
    if (l !== list.length)
        return true;

    for (k = 0; k < list.length; k++)
    {
        if (this.collection[list[k].id] == null)
            return true;
    }

    return false;
};

//Return Display instance
DownloadManager.prototype.get = function(element)
{
    var k;

    for (k in this.collection)
    {
        if (this.collection[k].element === element)
            return this.collection[k];
    }
};

//Clone download <template> element
function createElement(id)
{
    var e, f;

    e = document.querySelector("template.download");
    f = e.content.cloneNode(true).firstElementChild;
    f.id = id;

    return f;
}

//Convert number of bytes to human-readable form
function sizeHuman(bytes)
{
    var d, s;

    s = "";
    if (bytes > 0)
    {
        d = Math.floor(Math.log(bytes) / Math.log(1024));
        s = Math.ceil(bytes / Math.pow(1024, d)) + " " + " KMGTPEZY"[d] + "B";
    }

    return s;
}

function relayEvent(owner, event, selector, fn, capture) {
    var p, q;

    function callback(event)
    {
        var i, e, t, c, r;

        i = 0;
        t = event.target;
        c = event.currentTarget.querySelectorAll(selector);

        while ((e = c[i++]))
        {
            if (e === t || e.contains(t))
            {
                r = fn.call(e, {
                    target: t,
                    relay: e,
                    source: owner,
                    keyIdentifier: event.keyIdentifier,
                    keyCode: event.keyCode,
                    preventDefault: function() {
                        event.preventDefault();
                    },
                    stopPropagation: function() {
                        event.stopPropagation();
                    }
                });

                if (r == false)
                    event.preventDefault();

                return r;
            }
        }
    }

    q = typeof owner === "string" ? document.querySelectorAll(owner) : [owner];
    for (p = 0; p < q.length; p++)
        q[p].addEventListener(event, callback, capture);
}

chrome.runtime.getBackgroundPage(function(background)
{
    var manager = new DownloadManager();

    relayEvent(manager.node, "click", ".download", function(event)
    {
        var d;

        d = manager.get(event.relay);

        switch(event.target.className)
        {
            case "cancel":
                background.cancelDownload(d.id);
                break;
            case "retry":
                background.retryDownload(d.id);
                break;
            case "remove":
                background.removeDownload(d.id);
                break;
            default:
                break;
        }
    });

    window.setInterval(function()
    {
        var i, l;

        l = background.getAllDownloads();

        if (manager.needsUpdate(l))
            manager.clear();

        for (i = 0; i < l.length; i++)
            manager.update(l[i]);
    }, 1000);
});
