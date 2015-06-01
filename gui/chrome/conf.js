var conf_t = {
    toString: function(key, value)
    {
        var s = "-a";

        if ("max_speed" in this)
            s += " -s" + JSON.stringify(this.max_speed);

        if ("num_connections" in this)
            s += " -n" + JSON.stringify(this.num_connections);

        if ("output" in this)
            s += " -o" + JSON.stringify(this.output);

        if ("user_agent" in this)
            s += " -U" + JSON.stringify(this.user_agent);

        return s;
    }
};

function conf_init(conf)
{
    if ("max_speed" in localStorage)
        conf.max_speed = Number(localStorage.max_speed);

    if ("num_connections" in localStorage)
        conf.num_connections = Number(localStorage.num_connections);

    if ("output" in localStorage)
        conf.output = localStorage.output;

    if ("user_agent" in localStorage)
        conf.user_agent = localStorage.user_agent;
}
