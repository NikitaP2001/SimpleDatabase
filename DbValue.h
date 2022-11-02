#pragma once

#include <memory>
#include <vector>
#include <string>

class DbValue {

public:
        virtual bool SetValue(std::string) = 0;

        virtual std::string GetValue() const = 0;

        virtual ~DbValue() = default;
        virtual bool operator<(const DbValue *other) = 0;
        virtual bool operator==(const DbValue *other) = 0;
private:

};

std::shared_ptr<DbValue> getDbValue(std::string literal);

struct Column {
        std::string name;
        std::vector<std::shared_ptr<DbValue>> values; 
};


class DbInteger : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;

private:
        int m_value = 0;
};

class DbReal : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;
private:
        float m_value;
};

class DbChar : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;

private:
        char m_value;
};

class DbDate : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;

private:
        time_t m_time;
};

class DbDateLnvl : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;
private:

        time_t m_time1;
        time_t m_time2;
};

class DbString : public DbValue {

public:
        bool SetValue(std::string) override;

        std::string GetValue() const override;

        virtual bool operator<(const DbValue *other) override;
        virtual bool operator==(const DbValue *other) override;

private:
        std::string m_value;
};