## # Nginx config

``` bash
sudo mkdir -p /srv/tempserv/html
sudo cp -r /home/ribs/project_42/circle_5/tempserv/html/* /srv/tempserv/html/
sudo chown -R www-data:www-data /srv/tempserv/html
sudo chmod -R 755 /srv/tempserv/html
```

## # Security

- '**../**' can't do
- can't delete outside of a certain directory

## # Directory listing (by ChatGPT)

```c++
static void send_all(int sock, const char *data, size_t len) {
    while (len > 0) {
        ssize_t sent = send(sock, data, len, 0);
        if (sent <= 0) return;
        data += sent;
        len  -= sent;
    }
}

static void send_text(int sock, const std::string &s) {
    send_all(sock, s.c_str(), s.size());
}

static std::string html_escape(const std::string &s) {
    std::string out;
    for (size_t i=0;i<s.size();++i) {
        switch (s[i]) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;";  break;
            case '>': out += "&gt;";  break;
            default:  out += s[i];    break;
        }
    }
    return out;
}

static void send_directory_listing(int client, const std::string &path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        send_text(client, "HTTP/1.1 404 Not Found\r\n\r\nCannot open directory");
        return;
    }

    std::string body = "<html><body><h2>Index of " + html_escape(path) + "</h2><ul>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        body += "<li><a href=\"" + name + "\">" + html_escape(name) + "</a></li>";
    }
    closedir(dir);

    body += "</ul></body></html>";

    char header[256];
    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n\r\n",
            (unsigned long)body.size());

    send_text(client, header);
    send_text(client, body);
}
```

## # What POST request does

``` text
Client                                    Server
  |                                         |
  | --- POST /articles -------------------> |
  |     Content-Type: application/json      |
  |     {                                   |
  |       "title": "My First Article"       |
  |     }                                   |
  |                                         |
  |                                         |  [Server processes the request]
  |                                         |  [Creates new resource /articles/123]
  |                                         |
  |<-- 201 Created -------------------------|
  |    Content-Type: application/json       |
  |    Location: /articles/123              |
  |    {                                    |
  |      "id": 123,                         |
  |      "title": "My First Article",       |
  |      "status": "created"                |
  |    }                                    |
  |                                         |
```

## # Idea for configuration style structure (by ChatGPT)

``` c++
struct StatusInfo {
    std::string path;
    std::string message;
    std::string connection;
};

static const std::unordered_map<int, StatusInfo> statusTable = {
    {201, {"",                    " Created",                ""}},
    {204, {"",                    " No content",             ""}},
    {400, {"./errors/400.html",   " Bad request",            "close"}},
    {403, {"./errors/403.html",   " Forbidden",              ""}},
    {404, {"./errors/404.html",   " Not found",              ""}},
    {405, {"./errors/405.html",   " Method not allowed",     ""}},
    {500, {"./errors/500.html",   " Internal server error",  "close"}},
};

StatusInfo info = {"", " Ok", "keep-alive"}; // default

if (statusTable.count(sCode))
    info = statusTable.at(sCode);

path = info.path;
str  = info.message;
con  = info.connection;
```

## # CGI implementation

### / Handle pipe and fork
First we need to `fork()` the process after we encounter the [**cgi**](https://en.wikipedia.org/wiki/Common_Gateway_Interface) folder in the *URI*.  
Now we have:  
- Parent process -> waiting for child (continuing maybe ?)
- Child process -> executing the **cgi**

To make the output of the child be accessible to the parent we must use `pipe()`, then we add the *read* end of the pipe to the `epoll()` for listening. \*[not sure about the epoll part]  
When the child process finish we put the output in a file and send back the file to the client.  
The child must go into the correct directory before executing the **cgi**.
Parent continues and intercept child exit signal to process the data being sent in the pipe.  

Return:  
\[FALSE\]The function must return all of the **cgi** has print into the Client *sendBuff* variable.  
We need to parse the **cgi** response and send an HTTP header accordingly

### / POST method case 
In case of a **POST** method we need to redirect `STDIN_FILENO` of the child to something else ? don't know for now

### / Example of environment variable

``` text
REQUEST_METHOD=GET
QUERY_STRING=name=Bob&age=22
SCRIPT_NAME=/cgi-bin/test.cgi
SERVER_PROTOCOL=HTTP/1.1
SERVER_NAME=example.com
SERVER_PORT=80
REMOTE_ADDR=192.168.1.10
HTTP_USER_AGENT=Mozilla/5.0
HTTP_ACCEPT=*/*
```

## / To do
- [i] Change the Client class to have a cgi per client
- [ ] Fix the case where there is arbitrary spaces between value HTTP header
- [ ] Exit program when crtl-C not just passing the flag to 1 (making `execve()` impossible to fail)
- [ ] CGI Environment variable
