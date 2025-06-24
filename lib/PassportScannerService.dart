import 'dart:convert';
import 'dart:io';

class PassportScannerService {
  static const String _javaClassPath = './sino_passport/linux/target/distribution/manual_test.sh';

  static Future<PassportData?> scanPassport() async {
    try {
      final result = await Process.run(
        'java',
        ['-cp', '*', 'com.ktk.PassportScannerService'],
        workingDirectory: _javaClassPath,
      );

      if (result.exitCode == 0) {
        final jsonData = jsonDecode(result.stdout);

        if (jsonData['success'] == true) {
          return PassportData.fromJson(jsonData);
        } else {
          throw Exception(jsonData['error'] ?? 'Unknown error');
        }
      } else {
        throw Exception('Scanner process failed: ${result.stderr}');
      }
    } catch (e) {
      throw Exception('Failed to scan passport: $e');
    }
  }
}

class PassportData {
  final String passportNumber;
  final String expiry;
  final String issuingCountry;
  final String surname;
  final String firstName;
  final String englishName;
  final String gender;
  final String nationality;
  final String documentNumber;
  final String? imagePath;

  PassportData({
    required this.passportNumber,
    required this.expiry,
    required this.issuingCountry,
    required this.surname,
    required this.firstName,
    required this.englishName,
    required this.gender,
    required this.nationality,
    required this.documentNumber,
    this.imagePath,
  });

  factory PassportData.fromJson(Map<String, dynamic> json) {
    return PassportData(
      passportNumber: json['passportNumber'] ?? '',
      expiry: json['expiry'] ?? '',
      issuingCountry: json['issuingCountry'] ?? '',
      surname: json['surname'] ?? '',
      firstName: json['firstName'] ?? '',
      englishName: json['englishName'] ?? '',
      gender: json['gender'] ?? '',
      nationality: json['nationality'] ?? '',
      documentNumber: json['documentNumber'] ?? '',
      imagePath: json['imagePath'],
    );
  }
}