class PassportData {
  final String passportNumber;
  final String englishName;
  final String gender;
  final String birthDate;
  final String expiry;
  final String issuingCountry;
  final String surname;
  final String firstName;
  final String nationality;
  final String documentNumber;

  PassportData({
    this.passportNumber = '',
    this.englishName = '',
    this.gender = '',
    this.birthDate = '',
    this.expiry = '',
    this.issuingCountry = '',
    this.surname = '',
    this.firstName = '',
    this.nationality = '',
    this.documentNumber = '',
  });

  factory PassportData.fromMap(Map<String, dynamic> map) {
    return PassportData(
      passportNumber: map['passportNumber'] ?? '',
      englishName: map['englishName'] ?? '',
      gender: map['gender'] ?? '',
      birthDate: map['birthDate'] ?? '',
      expiry: map['expiry'] ?? '',
      issuingCountry: map['issuingCountry'] ?? '',
      surname: map['surname'] ?? '',
      firstName: map['firstName'] ?? '',
      nationality: map['nationality'] ?? '',
      documentNumber: map['documentNumber'] ?? '',
    );
  }

  Map<String, dynamic> toMap() {
    return {
      'passportNumber': passportNumber,
      'englishName': englishName,
      'gender': gender,
      'birthDate': birthDate,
      'expiry': expiry,
      'issuingCountry': issuingCountry,
      'surname': surname,
      'firstName': firstName,
      'nationality': nationality,
      'documentNumber': documentNumber,
    };
  }

  bool get isEmpty {
    return passportNumber.isEmpty &&
        englishName.isEmpty &&
        surname.isEmpty &&
        firstName.isEmpty;
  }

  @override
  String toString() {
    return 'PassportData{passportNumber: $passportNumber, name: $englishName, expiry: $expiry}';
  }
}