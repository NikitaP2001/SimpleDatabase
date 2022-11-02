#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "DbValue.h"

std::shared_ptr<DbValue> getDbValue(std::string literal)
{
        std::shared_ptr<DbValue> result;
        DbReal valReal;
        DbInteger valInt;
        DbDateLnvl valLnvl;
        DbDate valDate;
        DbChar valChr;
        DbString valStr;

        if (valInt.SetValue(literal))
                result = std::make_shared<DbInteger>(valInt);
        else if (valReal.SetValue(literal))
                result = std::make_shared<DbReal>(valReal);
        else if (valLnvl.SetValue(literal))
                result = std::make_shared<DbDateLnvl>(valLnvl);
        else if (valDate.SetValue(literal))
                result = std::make_shared<DbDate>(valDate);
        else if (valChr.SetValue(literal))
                result = std::make_shared<DbChar>(valChr);
        else if (valStr.SetValue(literal))
                result = std::make_shared<DbString>(valStr);
        else
                throw std::runtime_error("convert value failed");
        return result;
}

bool DbInteger::SetValue(std::string literal)
{
        bool status = false;
        try {
                int val = std::stoi(literal);
                if (std::to_string(val) == literal) {
                        m_value = val;
                        status = true;
                }
        } catch (std::exception &) {

        }
        return status;
}

std::string DbInteger::GetValue() const
{
        return std::to_string(m_value);
}

bool DbInteger::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbInteger *io = dynamic_cast<const DbInteger*>(other))
                result = m_value < io->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbInteger::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbInteger *io = dynamic_cast<const DbInteger*>(other))
                result = m_value == io->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}

bool DbReal::SetValue(std::string literal)
{
        bool status = false;
        size_t pos = 0;
        try {
                int val = std::stof(literal, &pos);
                if (pos == literal.size()) {
                        m_value = val;
                        status = true;
                }
        } catch (std::exception&) {

        }
        return status;
}

std::string DbReal::GetValue() const
{
        return std::to_string(m_value);
}

bool DbReal::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbReal *ro = dynamic_cast<const DbReal*>(other))
                result = m_value < ro->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}

bool DbReal::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbReal *ro = dynamic_cast<const DbReal*>(other))
                result = m_value == ro->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}

bool DbChar::SetValue(std::string literal)
{
        bool status = false;
        try {
                if (literal.size() == 1) {
                        m_value = literal.at(0);
                        status = true;
                }
        } catch (std::exception&)
        {
        }

        return status;
}

std::string DbChar::GetValue() const
{
        return std::string(1, m_value);
}

bool DbChar::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbChar *co = dynamic_cast<const DbChar*>(other))
                result = m_value < co->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}

bool DbChar::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbChar *co = dynamic_cast<const DbChar*>(other))
                result = m_value == co->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}

bool DbDate::SetValue(std::string literal)
{
        bool status = false;
        try {
                std::tm dt {};
                static const std::string dateTimeFormat{ "%Y-%m-%d" };
                std::stringstream ss{ literal };
                ss >> std::get_time(&dt, dateTimeFormat.c_str());

                if (!ss.fail()) {
                        m_time = std::mktime(&dt);
                        status = true;
                }
        } catch (std::exception &ex) {

        }
        return status;
}

std::string DbDate::GetValue() const
{
        std::stringstream ss;
        static const std::string dateTimeFormat{ "%Y-%m-%d" };
        ss << std::put_time(std::gmtime(&m_time), dateTimeFormat.c_str());
        return ss.str();
}

bool DbDate::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbDate *dto = dynamic_cast<const DbDate*>(other))
                result = m_time < dto->m_time;
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbDate::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbDate *dto = dynamic_cast<const DbDate*>(other))
                result = m_time == dto->m_time;
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbDateLnvl::SetValue(std::string literal)
{
        bool status = false;
        try {
                std::tm dt1 {};
                std::tm dt2 {};
                std::stringstream ss{ literal };
                static const std::string dateTimeFormat{ "%Y-%m-%d" };
                ss >> std::get_time(&dt1, dateTimeFormat.c_str());
                if (ss.get() == '-') {
                        ss >> std::get_time(&dt2, dateTimeFormat.c_str());
                        if (!ss.fail()) {
                                m_time1 = std::mktime(&dt1);
                                m_time2 = std::mktime(&dt2);
                                status = true;
                        }
                }

        } catch (std::exception &) {

        }
        return status;
}

std::string DbDateLnvl::GetValue() const
{
        std::stringstream ss;
        static const std::string dateTimeFormat{ "%Y-%m-%d" };
        ss << std::put_time(std::gmtime(&m_time1), dateTimeFormat.c_str());
        ss.put('-');
        ss << std::put_time(std::gmtime(&m_time2), dateTimeFormat.c_str());
        return ss.str();
}

bool DbDateLnvl::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbDateLnvl *dto = dynamic_cast<const DbDateLnvl*>(other))
                result = (m_time2 - m_time1) < (dto->m_time2 - dto->m_time1);
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbDateLnvl::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbDateLnvl *dto = dynamic_cast<const DbDateLnvl*>(other))
                result = (m_time2 - m_time1) == (dto->m_time2 - dto->m_time1);
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbString::SetValue(std::string literal)
{
        m_value = literal;
        return true;
}

std::string DbString::GetValue() const
{
        return m_value;
}

bool DbString::operator<(const DbValue *other)
{
        bool result = false;
        if (const DbString *so = dynamic_cast<const DbString*>(other))
                result = m_value < so->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}


bool DbString::operator==(const DbValue *other)
{
        bool result = false;
        if (const DbString *so = dynamic_cast<const DbString*>(other))
                result = m_value == so->m_value;
        else
                throw std::logic_error("wrong type compare");
        return result;
}