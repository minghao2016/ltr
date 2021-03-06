// Copyright 2011 Yandex

#ifndef LTR_PARAMETERS_CONTAINER_PARAMETERS_CONTAINER_H_
#define LTR_PARAMETERS_CONTAINER_PARAMETERS_CONTAINER_H_

#pragma warning(disable: 4290)

#include <rlog/rlog.h>

#include <list>
#include <stdexcept>
#include <string>
#include <map>

#include "ltr/interfaces/printable.h"
#include "ltr/utility/boost/any.h"
#include "ltr/utility/boost/shared_ptr.h"
#include "ltr/utility/boost/string_utils.h"


using std::list;
using std::string;
using std::logic_error;

using ltr::utility::Any;
using ltr::utility::any_cast;
using ltr::utility::bad_any_cast;
using ltr::utility::to_lower;

namespace ltr {
/**
* \brief This class is a storage for parameters of different types.
* Is able to store POD-type parameters and user-defined-type parameters.
* This class is needed for unified initialization in ltr-client.
* Class is case-insensetive of parameters names.
*
* \sa Parameterized
*/
class ParametersContainer: public Printable {
 public:
  typedef std::map<string, Any> StringAnyHash;
  typedef ltr::utility::shared_ptr<ParametersContainer> Ptr;

  ParametersContainer();
  ParametersContainer(const ParametersContainer& other);
  ParametersContainer& operator=(const ParametersContainer& other);
  /**
   * Erases all parameters from container
   */
  void Clear();
  /**
   * Gets iterator to the beginning of container
   */
  StringAnyHash::const_iterator begin() const {
    return name_value_hash_.begin();
  }
  /**
   * Gets iterator to the end of container
   */
  StringAnyHash::const_iterator end() const {
    return name_value_hash_.end();
  }
  /**
   * Returns true if the containter is empty,
   * otherwise returns false
   */
  bool empty() const {
    return name_value_hash_.empty();
  }

  template <class T>
  struct NameValue {
    NameValue(const string& parameter_name, const T& parameter_value)
        : name(to_lower(parameter_name))
        , value(to_lower(parameter_value)) { }
    string name;
    T value;
  };
  /**
   * Gets all parameters of given type
   * @result Returns list of parameters with given type
   */
  template <class T>
  list<NameValue<T> > getValuesByType() const {
    list<NameValue<T> > result;
    for (StringAnyHash::const_iterator iterator = name_value_hash_.begin();
         iterator != name_value_hash_.end();
         ++iterator) {
      T* value = any_cast<T>(&iterator->second);
      if (!value) {
        continue;
      }
      result.push_back(NameValue<T>(iterator->first, *value));
    }
    return result;
  }
  /**
   * Checks the existance of parameter with given name in container
  */
  bool Contains(const string& name) const;
  /**
   * Checks the coincidence of given type and type of parameter with
   * given name
  */
  template <class T>
  bool TypeCoincides(const string& name) const {
    const StringAnyHash::const_iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }
    return iterator->second.type() == typeid(T);
  }
  /**
   * Returns value of parameter with given name
   * If type of parameter and template type are not
   * the same throws
   */
  template<class T>
  T Get(const string& name, const Any& default_value = Any()) const
      throw(logic_error, std::bad_cast) {
    const StringAnyHash::const_iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end() && default_value.empty()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }
    const Any& found_value = (iterator != name_value_hash_.end() ?
                              iterator->second : default_value);

    try {
      return any_cast<T>(found_value);
    } catch (const bad_any_cast &exc) {
      rError("bad_any_cast");
      throw logic_error(string(exc.what()) +
                             "\nParameter name: " + name);
    }
  }
  /**
   * Returns value of parameter with given name
   * and casts it to DesiredType.
   */
  template<class StoredType, class DesiredType>
  DesiredType Get(const string& name, const Any& default_value = Any()) const
      throw(logic_error, std::bad_cast) {
    const StringAnyHash::const_iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end() && default_value.empty()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }
    const Any& found_value = (iterator != name_value_hash_.end() ?
                              iterator->second : default_value);

    try {
      const StoredType &value = any_cast<StoredType>(found_value);
      DesiredType desired_type_value = dynamic_cast<DesiredType>(value); //NOLINT
      return desired_type_value;
    } catch (const bad_any_cast &exc) {
      rError("bad_any_cast");
      throw logic_error(string(exc.what()) +
                             "\nParameter name: " + name);
    } catch (const std::bad_cast &exc) {
      throw logic_error(string(exc.what()) +
                             "\nParameter name: " + name);
    }
  }
  /**
   * Returns const reference to value of parameter with given name
   */
  template<class T>
  const T& GetRef(const string& name) const
      throw(logic_error, std::bad_cast) {
    const StringAnyHash::const_iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }

    try {
      return *any_cast<T>(&iterator->second);
    }
    catch (const bad_any_cast &exc) {
      rError("bad_any_cast");
      throw logic_error(string(exc.what()) +
                             "\nParameter name: " + name);
    }
  }
  /**
   * Returns reference to value of parameter with given name
   */
  template<class T>
  T& GetRef(const string &name)
      throw(logic_error, std::bad_cast) {
    const StringAnyHash::iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }

    try {
      return *any_cast<T>(&iterator->second);
    }
    catch (const bad_any_cast &exc) {
      rError("bad_any_cast");
      throw logic_error(string(exc.what()) +
                             "\nParameter name: " + name);
    }
  }


  /**
   * Sets given value for parameter with given name.
   * Adds new parameter if there is no such parameter name.
   * @param name - name of parameter to set/add
   * @param value - value to set/add
   */
  template<class T>
  void Set(const string &name, const T &value) {
    name_value_hash_[to_lower(name)] = value;
  }

  /**
   * Sets given value for parameter with given name.
   * Throws if there is no such parameter name.
   * @param name - name of parameter to set
   * @param value - value to set
   */
  template<class T>
  void SetExisting(const string& name, const T& value) {
    const StringAnyHash::iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator == name_value_hash_.end()) {
      throw logic_error("Parameter '" + name +  "' not defined");
    }
    iterator->second = value;
  }

  /**
   * Adds new parameter with given name and value.
   * Throws if container already has such parameter name.
   *
   * \param name - name of parameter to add
   * \param value - value to add.
   */
  void AddNew(const string& name, const Any& value) {
    const StringAnyHash::const_iterator iterator =
        name_value_hash_.find(to_lower(name));
    if (iterator != name_value_hash_.end()) {
      throw logic_error("There is already exist such parameter: " + name);
    }
    name_value_hash_[to_lower(name)] = value;
  }
  /**
   * Copies parameters from other container.
   * Throws if other container contains new parameters
   * or there is conflict of types.
   */
  void Copy(const ParametersContainer& parameters);
  /**
   * Print names and values of all parameters
   */
  virtual string toString() const;

 private:
  /**
   * Map from parameter name to parameter value
   */
  StringAnyHash name_value_hash_;
};
}


#endif  // LTR_PARAMETERS_CONTAINER_PARAMETERS_CONTAINER_H_
