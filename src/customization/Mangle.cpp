#include <Mangle.h>

#include <stdlib.h>


status_t
demangle_class_name(const char* name, BString& out)
{
// TODO: add support for template classes
//	_find__t12basic_string3ZcZt18string_char_traits1ZcZt24__default_alloc_template2b0i0PCccUlUl

	out = "";

#if __GNUC__ >= 4
	if (name[0] == 'N')
		name++;
	int nameLen;
	bool first = true;
	while ((nameLen = strtoul(name, (char**)&name, 10))) {
		if (!first)
			out += "::";
		else
			first = false;
		out.Append(name, nameLen);
		name += nameLen;
	}
	if (first)
		return B_BAD_VALUE;

#else
	if (name[0] == 'Q') {
		// The name is in a namespace
		int namespaceCount = 0;
		name++;
		if (name[0] == '_') {
			// more than 10 namespaces deep
			if (!isdigit(*++name))
				return B_BAD_VALUE;

			namespaceCount = strtoul(name, (char**)&name, 10);
			if (name[0] != '_')
				return B_BAD_VALUE;
		} else
			namespaceCount = name[0] - '0';

		name++;

		for (int i = 0; i < namespaceCount - 1; i++) {
			if (!isdigit(name[0]))
				return B_BAD_VALUE;

			int nameLength = strtoul(name, (char**)&name, 10);
			out.Append(name, nameLength);
			out += "::";
			name += nameLength;
		}
	}

	int nameLength = strtoul(name, (char**)&name, 10);
	out.Append(name, nameLength);
#endif

	return B_OK;
}
