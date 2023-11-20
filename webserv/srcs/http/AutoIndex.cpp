
#include "AutoIndex.hpp"

typedef std::string string;
typedef std::map<string, std::vector<string> > filelist;

// return a map --> {"dir": vector, "files": vector}
static filelist	get_files_and_directories(string path)
{
	DIR *dir;
	struct dirent *ent;
	filelist list;

	path = "./www/html" + path;

	debug_printing(E_DEBUG, "checking -> " + path);

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			switch (ent->d_type)
			{
				case DT_DIR:
					list["dir"].push_back(ent->d_name);
					break;

				default:
					list["files"].push_back(ent->d_name);
					break;
			}
		}
		closedir(dir);
	}
	else
		debug_printing(E_ERROR, "Cannot open dir at <" + path + ">");

	std::sort(list["dir"].begin(), list["dir"].end());
	std::sort(list["files"].begin(), list["files"].end());

	return list;
}

static string	remove_first_slashes(string path)
{

	if (path[0] != '/')
		return path;
	
	string new_path;
	for (int i = 1; i < (int) path.size(); i++)
	{
		if (i == (int) path.size() - 1 && path[i] == '/')
			break ;
		new_path.push_back(path[i]);
	}

	return remove_first_slashes(new_path);
}

static string	get_previous_folder(string path)
{
	string new_path;

	int slash_count = 0;
	int	i = 0;

	while (path[i])
	{
		if (i != (int) path.size() - 1 && path[i] == '/')
			slash_count += 1;
		i++;
	}

	i = 0;
	while (path[i])
	{
		if (path[i] == '/')
			slash_count--;
		if (slash_count == 0)
			break ;
		new_path.push_back(path[i++]);
	}

	return new_path;
}

static string	generateATagFileList(string base_path, filelist list)
{
	base_path = remove_first_slashes(base_path); 

	string folder_icon = "<li><img src=\"/assets/folder.png\">";
	string file_icon = "<li><img src=\"/assets/file.png\">";

	string starter = "<a href=\"";
	string middler = "\">";
	string closer = "</a></li>\r\n";

	string a_list = "";

	// set .. folder
	if (base_path.find("/"))
	{
		a_list += folder_icon + starter;
		a_list += "/" + get_previous_folder(base_path);
		a_list += middler + ".." + closer;
	}

	for (size_t i = 0; i < list["dir"].size(); i++) {
		if (list["dir"][i] == ".." || list["dir"][i] == ".")
			continue ;
        a_list += folder_icon + starter;
		a_list += "/" + base_path + "/" + list["dir"][i];
		a_list += middler;
		a_list += list["dir"][i];
		a_list += closer;
    }

	a_list += "<br>\n";

	for (size_t i = 0; i < list["files"].size(); i++)
	{
		debug_printing(E_DEBUG, base_path + "/" + list["files"][i]);
		a_list += file_icon + starter;
		a_list += "/" + base_path + "/" + list["files"][i];
		a_list += middler;
		a_list += list["files"][i];
		a_list += closer;
	}

	return a_list;
}

static string	getATags(string path)
{
	filelist list = get_files_and_directories(path);
	return generateATagFileList(path, list);
}

static string	generateAutoIndexBody(string path)
{
	string body = "";

	body += "<!DOCTYPE html>\r\n<html lang=\"en\">\r\n<head>\r\n";
	body += "<title >42 WebServer</title>\r\n</head>\r\n<body>\r\n<h2>";
	body += "Listing files and folders of " + path + "</h2>\r\n";
	body += "<div style=\"padding-left:30px;\">\r\n";

	body += getATags(path);

	body += "</div>\r\n</body>\r\n</html>\r\n\0";

	return body;
}

static void	write_to_file(string path, string & content)
{
	std::ofstream	file(path.c_str());
	file << content;
	file.close();
}

static void	print_debug_vars(sRequest *req)
{
	debug_printing(E_DEBUG, "Checking vars for autoindex");
	debug_printing(E_DEBUG, "fileName: " + req->fileName);
	if (req->bestLocation->autoindex)
		debug_printing(E_DEBUG, "bestLocation->autoindex: true");
	else
		debug_printing(E_DEBUG, "bestLocation->autoindex: false");
}

void	autoIndex(sConnection & conn)
{
	if (conn.getStatusCode() >= 300)
		return;

	sRequest *req = conn.request;
	sResponse *res = conn.response;
	
	print_debug_vars(req);
	if (!(req->fileName.empty() == true && req->bestLocation->autoindex == true))
		return ;
	else
		res->autoindex_response = true;

	std::cout << std::endl << GREEN_TEXT << "-AUTO-INDEX-" << RESET_COLOR << std::endl;

	static u_long file_id;
	file_id++;
	res->file_path = "temp_files/" + numToString(file_id) + ".index";

	debug_printing(E_DEBUG, "Auto-Index: Generating body");
	string	body = generateAutoIndexBody(req->path_file);

	debug_printing(E_DEBUG, "Auto-Index: Writing body to file <" + res->file_path + ">");
	write_to_file(res->file_path, body);

	debug_printing(E_DEBUG, "Auto-Index: Generating headers into response directly");

	res->header = "HTTP/1.1 200 OK\r\n";
	res->header += "Content-type:text/html; charset=utf-8\r\n";
	res->header += "Content-Lenght: " + numToString(body.size()) + "\r\n\r\n";

	struct stat fileStat;
	stat(res->file_path.c_str(), &fileStat);
	res->file_stat = fileStat;

	conn.setStatusCode(200);
	res->complete = true;
	debug_printing(E_DEBUG, "Auto-Index: All went well, or should");
}
