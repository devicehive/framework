#pragma once

#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux__
#include <unistd.h>
#endif // __linux__

class GPIOPin
{
public:
	enum DIRECTION
	{
		IN,
		OUT,
		NOT_SET
	};
	
	GPIOPin(int pin):m_pin(pin), m_dir(NOT_SET), m_exported(false)
	{
		m_exported = exportPin();
		std::stringstream str;
		str << "/sys/class/gpio/gpio" << pin << "/value";
		str >> val_path;
		str << "/sys/class/gpio/gpio" << pin << "/direction";
		str >> dir_path;
	}

	~GPIOPin()
	{
		if (m_exported)
		{
			unexportPin();
		}
	}

	void set(DIRECTION dir)
	{
		if (m_dir != dir)
		{
			m_dir = dir;
			std::ofstream diros(dir_path);
			switch (m_dir)
			{
			case IN:
				diros << "in";
				break;
			case OUT:
				diros << "out";
				break;
			default:
				break;
			}

			diros.close();
		}
	}
	
	inline int read()
	{
		int res =-1;
		if (!m_exported)
			return res;
		
		if (m_dir != IN)
			set(IN);

		FILE* file = fopen(val_path.c_str(), "r");
		if (file == NULL)
			return res;

		char buf;
		if (fread(&buf, 1, 1, file))
			res = int(buf)-48;//"0"
		fclose(file);
		return res;
	}

	bool write(int val)
	{
		bool res = false;
		if (!m_exported)
			return res;

		if (m_dir != OUT)
			set(OUT);
		
		std::ofstream valfs(val_path);
		if (valfs.good())
		{
			valfs << val;
			res = true;
		}

		valfs.close();
		return res;
	}

	bool exportPin()
	{
		std::ofstream exportfs("/sys/class/gpio/export");
		if (!exportfs.good())
			return false;

		exportfs << m_pin;
		exportfs.close();
		return true;
	}

	bool unexportPin()
	{
		std::ofstream unexportfs("/sys/class/gpio/unexport");
		if (!unexportfs.good())
			return false;

		unexportfs << m_pin;
		unexportfs.close();
		return true;
	}

private:

	int m_pin;
	DIRECTION m_dir;
	std::string val_path, dir_path;
	bool m_exported;
};
