#include "all.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "GlobalConfig.hpp"

void get_line(std::ifstream &file, std::string &line)
{
	std::getline(file, line);
	trim_line(line);
	if (!file.eof() && file.fail())
		throw std::runtime_error("error while reading configuration file" + static_cast<std::string>(strerror(errno)));

}

void process_loc(std::ifstream &file, std::fstream &temp, std::string &line)
{
	trim_line(line);
	if (line[line.size() - 1] != '{')
		throw std::runtime_error("error in configuration file's syntax" + static_cast<std::string>(strerror(errno)));
	temp << line.substr(0, line.size() - 1) << std::endl; // pour enlever le ; à la fin
	while (1)
	{
		get_line(file, line);
		if (file.eof()) // attention ici
			throw std::runtime_error("error in configuration file's syntax" + static_cast<std::string>(strerror(errno)));
		if (line[0] == '#' || line == "")
			continue;
		if (line == "}")
			break;
		if (line[line.size() - 1] != ';')
			throw std::runtime_error("error in configuration file's syntaxe1");
		temp << line.substr(0, line.size() - 1) << std::endl; // pour enlever le ; à la fin
	}
}

void process_serv(std::ifstream &file, std::fstream &temp, std::string &line)
{
	int loc = 0;

	line = line.substr(6);
	trim_line(line);
	temp << "server" << std::endl;
	if (line[line.size() - 1] != '{')
		throw std::runtime_error("error in configuration file's syntax");
	while (1)
	{
		get_line(file, line);
		if (file.eof())
			throw std::runtime_error("error in configuration file's syntax" + static_cast<std::string>(strerror(errno)));
		if (line[0] == '#' || line == "")
			continue;
		if (line == "}")
			break;
		if (loc == 0 && !(line.substr(0, 8) == "location")) // il s'agit d'une directive de serveur
		{
			if (line[line.size() - 1] != ';')
				throw std::runtime_error("error in configuration file's syntaxe2");
			temp << line.substr(0, line.size() - 1) << std::endl; // pour enlever le ; à la fin
		}
		else if (line.substr(0, 8) == "location") // il s'agit d'un serveur
		{
			loc++;
			process_loc(file, temp, line);
		}
		else // directive apres le serveur
			throw std::runtime_error("" + static_cast<std::string>(strerror(errno)));
		// std::cout << "bem" << line << std::endl; // pour enlever le ; à la fin
		// temp << "bem" << line.substr(0, line.size() - 1) << std::endl; // pour enlever le ; à la fin
	}
}

void first_parse(std::ifstream &file, std::fstream &temp)
{
	std::string line;
	int serv = 0;

	temp.open("temp.txt", std::ios::out | std::ios::trunc);
	if (!temp.is_open())
		throw std::runtime_error("could not open temp file: " + static_cast<std::string>(strerror(errno)));
	while (1)
	{
		get_line(file, line);
		if (file.eof()) // attention ici
			break;
		// if (line[0] == '#' || line == "")
		if (line[0] == '#' || line == "" || line == ";")
			continue;
		if (serv == 0 && !(line.substr(0, 6) == "server")) // il s'agit d'une directive globale
		{
			if (line[line.size() - 1] != ';')
				throw std::runtime_error("error in configuration file's syntaxe3");
			temp << line.substr(0, line.size() - 1) << std::endl; // pour enlever le ; à la fin
		}
		else if (line.substr(0, 6) == "server") // il s'agit d'un serveur
		{
			serv++;
			process_serv(file, temp, line);
		}
		else // directive apres le serveur
			throw std::runtime_error("error in configuration file's syntaxe4");
	}
}

GlobalConfig parseConfig(std::string config_path)
{
	std::ifstream file(config_path.c_str());
	std::fstream temp;
	if (!file)
		throw std::runtime_error("could not open configuration file: " + static_cast<std::string>(strerror(errno)));
	first_parse(file, temp);
    // perror("no");
	temp.close();
    temp.open("temp.txt", std::ios::in);
	// perror("trait");
	GlobalConfig config(temp);
	// config.print_config();
	temp.close();
	return (config);
}

int main(int ac, char **av)
{
	std::string config_path;
	if (ac > 1)
		config_path = av[1];
	else
		config_path = "default_conf.conf";
	try
	{
		GlobalConfig config = parseConfig(config_path);
		Server serv(&config);
		signal(SIGINT, Server::handleSigint);
		serv.launch();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}
