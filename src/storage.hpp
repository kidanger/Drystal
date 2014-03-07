#pragma once

class Storage
{
public:
    void store(const char* key, const char* value);
    const char* fetch(const char* key) const;
};

