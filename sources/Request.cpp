#include "all.hpp"
#include "Request.hpp"

/*
static bool startWith( std::string& str, std::string prefix ) {
	if (prefix.size() > str.size())
		return false;
	return str.compare(0, prefix.size(), prefix) == 0;
}
*/

// NB = pas trop de protection sur les getline

void Request::getWriteLocation(int *j, std::string &pathfile)
{
	Directive	directive;
	std::vector<std::string> arg;
	std::string uri;
	size_t size = 0;

	DEBUG_MSG("Paht " << pathfile);
	for (std::vector<LocationConfig>::size_type i = 0; i < _locs.size(); i++)
	{
		directive = getDirective("root", _locs[i].getDir());
		arg = directive.getArg();
		uri = _locs[i].getUri();
		DEBUG_MSG("uri " << locs[i].getUri()<< " et i =" << i);
		if (pathfile.rfind(uri, 0) == 0)
		{
			DEBUG_MSG("uri " << _locs[i].getUri()<< "P");
			// DEBUG_MSG("arg[0].size() " << arg[0].size());
			DEBUG_MSG("(*size) " << size);
			if (uri.size() > size)
			// if (arg[0].size() > size)
			{
				// perror("omg");
				size = arg[0].size();
				(*j) = i;
			}
		}
	}
	DEBUG_MSG("j final " << (*j));
}

void Request::checkRedirAndMethod()
{
	// std::vector<LocationConfig> locs = _serv.getLocation();
	int j = -1;
	Directive dir;
	std::vector<std::string> arg;
	int flag = 0;
	int code;

	getWriteLocation(&j, _pathfile);
	std::cout << "j = " << j <<std::endl;
	if (j != -1)
	{
		for (std::vector<Directive>::size_type i = 0; i < _locs[j].getDir().size(); i++)
		{
			dir = _locs[j].getDir()[i];
			if (dir.getName() == "allow_methods")
			{
				flag = 1;
				perror("exper");
				arg = dir.getArg();
				for (std::vector<std::string>::size_type k = 0; k < arg.size(); k++)
				{
					if (arg[k] == _action)
						flag = 0;
				}
			}
			if (dir.getName() == "return")
			{
				perror("riment");
				arg = dir.getArg();
				code = std::atoi(arg[0].c_str());
				_sCode = code;
				_location = arg[1];
				return ;
			}
		}
	}
	if (flag == 1) //methode pas autorisee
		_sCode = 405;
}

void Request::getPath(std::string &pathfile)
{
	// std::vector<LocationConfig> locs = _serv.getLocation();
	Directive	directive;
	std::vector<std::string> arg;
	int j =  -1;

	// std::cout << "coddddddddddeeeeee =" << _sCode << std::endl;
	// std::cout << "pathfile = " << pathfile << std::endl;
	getWriteLocation(&j, pathfile);
	if (j != -1)
		directive = getDirective("root", _locs[j].getDir());
	else
		directive = getDirective("root", _serv.getDir());
	arg = directive.getArg();
	// if (pathfile[0] != '/') //inutile ??
	// 	pathfile.insert(0, "/"); // inutile ??
	std::cout << "codeeeeee =" << _sCode << std::endl;
	if (_sCode < 300)
		pathfile.insert(0, arg[0]);
	// DEBUG_MSG("400 = " << _errorPath[400]);
	// DEBUG_MSG("403 = " << _errorPath[403]);
	// DEBUG_MSG("404 = " << _errorPath[404]);
	// DEBUG_MSG("405 = " << _errorPath[405]);
	// DEBUG_MSG("500 = " << _errorPath[500]);
}

// static std::string getFile( std::string &pathfile, size_t* fileLength ) {
std::string Request::getFile( std::string &pathfile, size_t* fileLength ) {
	std::fstream	fs;
	std::string		tmp, res;

	DEBUG_MSG("first path to get " << pathfile);
	getPath(pathfile);
	fs.open(pathfile.c_str(), std::ios::in);
	if (fs.is_open()) {
		while (std::getline(fs, tmp)) {
			res += tmp;
			tmp.clear();
		}
		fs.close();
		*fileLength = res.size();
	}
	else {
		DEBUG_MSG("ERROR: Couldn't open file [" << pathfile << "]");
		res.clear();
	}
	DEBUG_MSG("after first path to get " << pathfile);
	return res;
}

/* This function can be reduce with a hash table */
std::string Request::ifError( std::string& path, std::string& con, int sCode ) {
	std::string str;

	switch (sCode)
	{
	case 201:
		// path = newly created ressource
		str = " Created"; break;
	case 204:
		// no path
		str = " No content"; break;
	case 301:
		path = ROOT_STR + REDIR_301;
		str = " Moved Permanently"; break;
	case 302:
		path = ROOT_STR + REDIR_302;
		str = " Found"; break;
	case 307:
		path = ROOT_STR + REDIR_307;
		str = " Temporary Redirect"; break;
	case 308:
		path = ROOT_STR + REDIR_308;
		str = " Permanent Redirect"; break;
	case 400:
		path = _errorPath[400];
		str = " Bad request"; con = "close"; break;
	case 403:
		path = _errorPath[403];
		str = " Forbidden"; break;
	case 404:
		path = _errorPath[404];
		str = " Not found"; break;
	case 405:
		path = _errorPath[405];
		str = " Method not allowed"; break; // Connection normally keep-alive
	case 500:
		path = _errorPath[500];
		str = " Internal server error"; con = "close"; break;
	default:
		str = " Ok"; con = "keep-alive"; break;
	}

	return str;
}

// void Request::checkPath( std::string pathfile, size_t& eCode ) {
// 	struct stat fileStat;

// 	getPath(pathfile);
// 	std::cout << "bef code = " << eCode << std::endl;
// 	DEBUG_MSG("after first getPaht = " << pathfile);
// 	if (_sCode != 200)
// 		return;
// 	if (stat(pathfile.c_str(), &fileStat) < 0){
// 		eCode = 404;
// 		DEBUG_MSG("Path does not exist: " << pathfile);
// 		return ;
// 	}
// 	if ((pathfile.c_str(), F_OK) < 0) {
// 		eCode = 404;
// 	}
// 	else if ((pathfile.find("/errors/")) != std::string::npos) {
// 		eCode = 403;
// 	}
// 	std::cout << "code = " << eCode << std::endl;
// }

void Request::checkPath( std::string pathfile, size_t& eCode ) {
	struct stat fileStat;

	getPath(pathfile);
	DEBUG_MSG("after first getPaht = " << pathfile);
	if (_sCode != 200)
		return;
	if (stat(pathfile.c_str(), &fileStat) < 0){
		eCode = 404;
		DEBUG_MSG("Path does not exist: " << pathfile);
		return ;
	}
	if (S_ISREG(fileStat.st_mode)) //c'est un fichier
	{}
	else if (S_ISDIR(fileStat.st_mode)) // c'est un dossier
	{
		// if ((pathfile.c_str(), F_OK) < 0) {
		eCode = 404;
		// }
	}
	else //c'est un autre type de fichier
	{
		eCode = 403; // Accès non autorisé à ce type
        DEBUG_MSG("Path is neither file nor directory: " << pathfile);
	}
	if ((pathfile.find("/errors/")) != std::string::npos) {
		eCode = 403;
	}
}

void Request::setErrorPath() // ICI CHANGER POUR AJOUTER LE LOCATION SI BESOIN
{
	// std::vector<LocationConfig> locs = _serv.getLocation();
	std::vector<Directive> dirs;
	std::string name;
	std::vector<std::string> arg;
	Directive root;

	// (void) j;
	if (_locationIndex == -1)
	{
		dirs = _serv.getDir();
		root = getDirective("root", dirs);
	}
	else
	{
		dirs = _locs[_locationIndex].getDir();
		root = getDirective("root", dirs);
	}
	DEBUG_MSG(" _locationIndex = " << _locationIndex);
	for(std::vector<Directive>::size_type i = 0; i < dirs.size(); i++)
	{
		name = dirs[i].getName();
		arg = dirs[i].getArg();
		if (name == "error_page" && arg[0] == "500")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[500] = root.getArg()[0] + arg[1]; // a_locationIndexouter ici le root
				else
					_errorPath[500] = root.getArg()[0] + _locs[_locationIndex].getUri() + arg[1]; //a_locationIndexouter le root.getArg()[0]
			}
		}
		if (name == "error_page" && arg[0] == "400")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[400] = root.getArg()[0] + arg[1];
				else
					_errorPath[400] = root.getArg()[0] + _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "403")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[403] = root.getArg()[0] + arg[1];
				else
					_errorPath[403] = root.getArg()[0] + _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "404")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[404] = root.getArg()[0] + arg[1];
				else
					_errorPath[404] = root.getArg()[0] + _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "405")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[405] = root.getArg()[0] + arg[1];
				else
					_errorPath[405] = root.getArg()[0] + _locs[_locationIndex].getUri() + arg[1];
			}
		}
	}
	if (_locationIndex == -1 && _errorPath.find(405) == _errorPath.end())
	    _errorPath[405] = ROOT_STR + ERROR_405; //chemin entier
	if (_locationIndex == -1 && _errorPath.find(400) == _errorPath.end())
	    _errorPath[400] = ROOT_STR + ERROR_400;
	if (_locationIndex == -1 && _errorPath.find(403) == _errorPath.end())
	    _errorPath[403] = ROOT_STR + ERROR_403;
	if (_locationIndex == -1 && _errorPath.find(404) == _errorPath.end())
	    _errorPath[404] = ROOT_STR + ERROR_404;
	if (_locationIndex == -1 && _errorPath.find(500) == _errorPath.end())
	    _errorPath[500] = ROOT_STR + ERROR_500;
	DEBUG_MSG("400 = " << _errorPath[400]);
	DEBUG_MSG("403 = " << _errorPath[403]);
	DEBUG_MSG("404 = " << _errorPath[404]);
	DEBUG_MSG("405 = " << _errorPath[405]);
	DEBUG_MSG("500 = " << _errorPath[500]);
}

Request::Request( Client& cli ): _cli(cli), _serv(_cli.getServ()), _locationIndex(-1) {
	std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp;

	_locs = _serv.getLocation();
	_sCode = 200;
	_connection = "keep-alive";
	cli.setCon(true);
	std::getline(ss, tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp)) {
		rawParam << tmp;
	}
	while (std::getline(rawParam, key, ':') && rawParam >> value) {
		remove_blank(value);
		_reqParam[key] = value;
	}
	setErrorPath();
}

Request::Request( const Request& cpy ): _cli(cpy._cli), _serv(cpy._serv), _locationIndex(cpy._locationIndex) {
	_reqParam = cpy._reqParam;
	_errorPath = cpy._errorPath;
	_locs = cpy._locs;
}

Request& Request::operator=( const Request& other ) {
	if (this != &other)
	{
		_reqParam = other._reqParam;
		_errorPath = other._errorPath;
		_locationIndex = other._locationIndex;
		_locs = other._locs;
	}
	return *this;
}

Request::~Request() {}

std::string Request::getPathFile( void ) const {
	return _pathfile;
}

std::string Request::getAction( void ) const {
	return _action;
}

std::map<std::string, std::string> Request::getSpec( void ) const {
	return _reqParam;
}

int Request::getsCode( void ) const {
	return _sCode;
}

bool Request::is_cgi( std::string cgiFolder ) {

	if (_pathfile.find(cgiFolder) != std::string::npos)
		return true;
	else
		return false;
}

void Request::parseHttp( void ) {
	std::string	tmp;

	std::getline(_rawHttp, _action, ' ');
	remove_blank(_action);

	if (_action != "GET" &&
			_action != "POST" &&
			_action != "DELETE") {
		_sCode = 405;
	}

	std::getline(_rawHttp, _pathfile, ' ');
	remove_blank(_pathfile);

	// Only for test purpose
	std::string pathWithoutQuery(_pathfile);
	size_t end;
	if ( (end = _pathfile.find('?')) != std::string::npos )
		pathWithoutQuery = _pathfile.substr(0, end);


	if (_pathfile.empty())
		_sCode = 400; // ici plutot
	// else if (IsRedir() == true)
	// { }
	// else if (IsMethodAllowed() == false)
	// 	_sCode = 405;
	else {
		checkPath(pathWithoutQuery, _sCode);
	}

	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty()) {
		if (tmp != "HTTP/1.1") {
			_sCode = 400;
		}
	}
	else
		_sCode = 400;
	DEBUG_MSG("path at end of parse = " + _pathfile);
	std::cout << "ode =" << _sCode << std::endl;
}

void Request::fGet( void ) {
	DEBUG_MSG("GET request");
	ifError(_pathfile, _connection, _sCode);
	std::cerr << "code = " << _sCode << ";, path = " << _pathfile << std::endl;
	_file = getFile(_pathfile, &_fileLength);
	if (_file.empty()) {
		_sCode = 403;
		ifError(_pathfile, _connection, _sCode);
		_file = getFile(_pathfile, &_fileLength);
	}
	std::cerr << "apres = code = " << _sCode << ";, path = " << _pathfile << std::endl;
}

void Request::fPost( void ) {
	DEBUG_MSG("POST request");
	_sCode = 201;
	fGet();
}

void Request::fDelete( void ) {
	DEBUG_MSG("DELETE request");
	getPath(_pathfile);
	std::cout << "delete pathfile : " << _pathfile << std::endl;
	if (access(_pathfile.c_str(), W_OK) == 0) {
		std::remove(_pathfile.c_str());
		_sCode = 204;
		return;
	}
	_sCode = 403;
	fGet();
}

void Request::handleAction( std::string action ) {
	std::string check[3] = {"GET", "POST", "DELETE"};
	void (Request::*f[3]) ( void ) = {&Request::fGet, &Request::fPost, &Request::fDelete};

	DEBUG_MSG("CHOOSE Method " << _action);
	DEBUG_MSG("path = " + _pathfile);
	std::cout << "codi = " << _sCode << std::endl;
	int i;
	if (_sCode == 200) {
		for (i = 0; i < 3; i++) {
			if (check[i] == action) {
				(this->*f[i])();
				break;
			}
		}
		// Next statement useless ?
		if (i == 3) {
			DEBUG_MSG("Not known method");
			fGet();
		}
	}
	else {
		DEBUG_MSG("Error code: " << _sCode);
		fGet();
	}
}

std::string Request::makeResponse( void ) {
	std::ostringstream mess;
	// ServerConfig conf = _cli.getServ();

	mess << "HTTP/1.1" << " " << _sCode << ifError(_pathfile, _connection, _sCode) << ENDLINE;
	mess << "Date: " << date(HTTP) << ENDLINE;
	mess << "Server: " << _serv.getIp() << ":" << _serv.getPort() << ENDLINE; // Modify according configuration file / fetch the host of the request
	// mess << "Server: " << "localhost" << ENDLINE; // Modify according configuration file / fetch the host of the request
	mess << "Connection: " << _connection << ENDLINE; // Modify either the connection need to be maintained or not
	if (_sCode > 300 && _sCode < 400)
		mess << "Location: " << _location << ENDLINE;
	if (_sCode != 204) {
		mess << "Content-Type: " << "text/html" << ENDLINE; // Modify according to file
		mess << "Content-Length: " << _fileLength << ENDLINE;
		mess << ENDLINE;
		mess << _file;
	}
	else
		mess << ENDLINE;
	std::cerr << "message envopye = " << mess.str() << std::endl;
	_cli.setSendBuff(mess.str());
	if (_connection == "keep-alive")
		_cli.setCon(true);
	else
		_cli.setCon(false);
	return _pathfile;
}
