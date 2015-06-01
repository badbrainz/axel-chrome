var rx_size = /^File size: ([0-9]+) bytes/;
var rx_name = /^Opening output file (.*)/;
var rx_done = /Downloaded ([0-9]+) in ([0-9:]+) seconds. \([\s0-9.A-Z\/s]+\)/;
var rx_progress = /\[\s*([0-9]+)%\] \[[\s0-9#.]+\] \[\s*([0-9A-Z.]+)\/s\] \[\s*([0-9hd:]+)\]/;
var instance_count = 0;

var url_t = {
    toString: function()
    {
        var s = "";

        if (this.text)
            s += JSON.stringify(this.text);

        if (this.next != null)
            s += " " + this.next;

        return s;
    }
};

var axel_t = {
    toJSON: function()
    {
        return {
            id: this.id,
            url: this.url,
            filename: this.filename,
            size: this.size,
            bytes_done: this.bytes_done,
            ready: this.ready,
            message: this.message,
        };
    },
    toString: function()
    {
        return JSON.stringify(this, [
            "id",
            "url",
            "filename",
            "size",
            "bytes_done",
            "ready",
            "message"
        ]);
    }
};

function axel_new(conf, urls)
{
    var axel, i, u;

    axel = Object.create(axel_t);
    axel.id = instance_count++;
    axel.port = null;
    axel.conf = conf;
    axel.filename = "";
    axel.size = 0;
    axel.bytes_done = 0;
    axel.message = "";
    axel.url = Object.create(url_t);

    u = axel.url;
    for (i = 0; i < urls.length; i++)
    {
        u.text = urls[i];
        if (i < urls.length - 1)
        {
            u.next = Object.create(url_t);
            u = u.next;
        }
    }

    if (!axel.url.text)
    {
        axel.ready = -1;
    }

    return axel;
}

function axel_start(axel)
{
    axel.port = chrome.runtime.connectNative(HOST_ID);

    axel.port.onMessage.addListener(function(message)
    {
        var m;

        if ((m = message.match(rx_size)))
        {
            axel.size = Number(m[1]);
        }
        else if ((m = message.match(rx_name)))
        {
            axel.filename = m[1];
        }
        else if ((m = message.match(rx_done)))
        {
            axel.bytes_done = Number(m[1]);
        }

        axel.message = message;
    });

    axel.port.onDisconnect.addListener(function()
    {
        axel_close(axel);
    });

    axel.port.postMessage(HOST_NAME + " " + axel.conf + " " + axel.url);

    axel.ready = 0;
}

function axel_close(axel)
{
    if (axel.port)
    {
        axel.port.disconnect();
        axel.port = null;
    }

    if (axel.size === 0 || axel.bytes_done < axel.size)
    {
        axel.ready = -1;
    }
    else
    {
        axel.ready = 1;
    }
}

