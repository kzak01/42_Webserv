#ifndef BODYREAD_HPP
# define BODYREAD_HPP
#include "../../libraries/types.hpp"
#include "../../libraries/includes.hpp"
#include "../../libraries/classes.hpp"
# include "../../srcs/utils.hpp"

class BodyRead
{
private:
	std::string		_body_path;
	
	int				_chunked_transfer(sConnection &ref);
	int				_parse_body(sConnection &ref);
	int				_multipart_formData(sConnection &ref, std::string separator);
	int 			_parse_parts(sConnection &ref, std::vector<std::string> &parts);
	//ERROR HANDLING
	int				_exitError(sConnection &ref, int Error);
	int				_checkSize(sConnection &ref, int BufferSize);

	// UTILITIES
	int				_printSocketBoundaries(sConnection &ref, int FLAG);
public:
	BodyRead(sConnection &ref);
	~BodyRead();
};

#endif
