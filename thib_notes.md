## # Nginx config

``` bash
sudo mkdir -p /srv/tempserv/html
sudo cp -r /home/ribs/project_42/circle_5/tempserv/html/* /srv/tempserv/html/
sudo chown -R www-data:www-data /srv/tempserv/html
sudo chmod -R 755 /srv/tempserv/html
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
---

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
---

## # CGI implementation

### /Plan
First we need to `fork()` the process after we encounter the '**.cgi**' extension file.  
Now we have:  
- Parent process -> waiting for child (continuing maybe ?)
- Child process -> executing the '**.cgi**' file

To make the output of the child be accessible to the parent we must use `pipe()`, then we add the *read* end of the pipe to the `epoll()` for listening.  
When the child process finish we put the output in a file and send back the file to the **client**.

### /Execution