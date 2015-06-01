// For restoring html values
var options = {
    output: localStorage.output,
    num_connections: localStorage.connections,
    max_speed: localStorage.max_speed,
    user_agent: localStorage.user_agent
};

function loadOptions()
{
    var c;

    c = Object.create(conf_t);
    conf_init(c);

    if ("output" in c)
        document.getElementById("output").value = c.output;

    if ("num_connections" in c)
        document.getElementById("num_connections").value = c.num_connections;

    if ("max_speed" in c)
        document.getElementById("max_speed").value = c.max_speed;

    if ("user_agent" in c)
        document.getElementById("user_agent").value = c.user_agent;
}

function updateLocalStorage()
{
    var k;

    for (k in options)
    {
        if (options[k] != null)
        {
            localStorage[k] = options[k];
        }
        else
        {
            delete localStorage[k];
        }
    }
}

document.body.addEventListener("blur", function(event)
{
    var t, w;

    event.target.value = event.target.value.trim();

    t = event.target;

    if (t.validity.valid === true)
    {
        if (t.value != "")
        {
            w = t.type === "number" ? t.valueAsNumber : t.value;
        }
        options[t.id] = w;
        updateLocalStorage();
    }
    else
    {
        t.value = options[t.id];
    }
}, true);

loadOptions();
