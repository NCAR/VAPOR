#ifndef VAPORTABLE_H
#define VAPORTABLE_H

#include <QWidget>
#include <sstream>

struct Value;

// class VaporTable
//
class VaporTable : public QWidget {
 Q_OBJECT

public:

 enum MutabilityFlags {
	MUTABLE = (1u << 0),
	IMMUTABLE = (1u << 1),
 };

 // The ValidatorTypeFlag will be used to apply validators
 // to the cells that are regenerated on each Update call.
 //
 // Can this be replaced with the template info in Update?
 //
 enum ValidatorFlags {
	INT = (1u << 0),
	DOUBLE = (1u << 1),
	STRING = (1u << 2),
 };

 VaporTable(QTableWidget* table,
			bool lastRowIsCheckboxes = false,
			bool lastColIsCheckboxes = false);

 void Update(
	int rows, int columns,
	std::vector<int> values,
	std::vector<std::string> rowHeaders = std::vector<std::string>(),
	std::vector<std::string> colHeaders = std::vector<std::string>());
 void Update(
	int rows, int columns,
	std::vector<double> values,
	std::vector<std::string> rowHeaders = std::vector<std::string>(),
	std::vector<std::string> colHeaders = std::vector<std::string>());
 void Update(
	int rows, int columns,
	std::vector<std::string> values,
	std::vector<std::string> rowHeaders = std::vector<std::string>(),
	std::vector<std::string> colHeaders = std::vector<std::string>());

 void Reinit(ValidatorFlags vFlags, MutabilityFlags mFlags);
 
 Value GetValue(int row, int col);

 // Dump all values in the table back to the user
 void GetValues(std::vector<std::string> &vec);
 void GetValues(std::vector<int> &vec);
 void GetValues(std::vector<double> &vec);

 void setCheckboxesInFinalColumn(bool enabled);
 void setCheckboxesInFinalRow(bool enabled);

 // I think we may need something like this.  TBD...
 void setCellMutability(int row, int col);

public slots:
 void emitValueChanged();
 //void emitCellClicked();

signals:

 void valueChanged(int row, int col);
 void cellClicked(int row, int col);

private:
 void emitCellClicked(QObject* object);
 bool eventFilter(QObject* object, QEvent* event);

 std::vector<std::string> convertToString(std::vector<int> values);
 
 std::vector<std::string> convertToString(std::vector<double> values);

 QLineEdit* createLineEdit(QString val);

 void setHorizontalHeader(std::vector<std::string> header);

 void setVerticalHeader(std::vector<std::string> header);

 void setValidator(QLineEdit *edit);
 
 void setTableCells(std::vector<std::string> values);

 void addCheckboxes(std::vector<std::string> values);

 void addCheckbox(int row, int column, bool checked=false);

 bool isValueChecked(std::vector<std::string> values, int index);

 bool _lastRowIsCheckboxes;
 bool _lastColIsCheckboxes;
 QTableWidget *_table;

 MutabilityFlags _mutabilityFlags;
 ValidatorFlags _validatorFlags;
};

// A way to return a generic built-in type, so that the user can do:
// int myVal = vaporTable->GetValue(0,0);
// double myVal = vaporTable->GetValue(0,0);
// std::string myVal = vaporTable->GetValue(0,0);
struct Value{
	std::string _value;

	template<typename T>
	operator T() const   //implicitly convert into T
	{
	   std::stringstream ss(_value);
	   T convertedValue;
	   if ( ss >> convertedValue ) return convertedValue;
	   else throw std::runtime_error("conversion failed");
	}
};

#endif
