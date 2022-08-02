#pragma once

struct InvalidAPIObject : public std::invalid_argument
{
	using std::invalid_argument::invalid_argument;
};

struct InvalidAPIArgument : public std::invalid_argument
{
	using std::invalid_argument::invalid_argument;
};

struct InvalidPipeline : public std::invalid_argument
{
	using std::invalid_argument::invalid_argument;
};