package com.ktk;

import com.sun.jna.platform.win32.WTypes;
import com.sun.jna.ptr.IntByReference;
import com.google.gson.Gson; // Add gson dependency to pom.xml
import java.util.HashMap;
import java.util.Map;

public class PassportScannerService {

    public static void main(String[] args) {
        Map<String, Object> result = scanPassport();

        // Output JSON to stdout (Flutter will capture this)
        Gson gson = new Gson();
        System.out.println(gson.toJson(result));
    }

    public static Map<String, Object> scanPassport() {
        Map<String, Object> result = new HashMap<>();

        try {
            CIDCardRecog idCardLib = CIDCardRecog.INSTANCE;

            // Initialize device
            WTypes.LPWSTR userID = new WTypes.LPWSTR("426911010110763248");
            WTypes.LPWSTR dir = new WTypes.LPWSTR(".");
            int initRes = idCardLib.InitIDCard(userID, 0, dir);

            if (initRes != 0) {
                result.put("success", false);
                result.put("error", "Failed to initialize device");
                return result;
            }

            // Set config
            WTypes.LPWSTR configFile = new WTypes.LPWSTR("./IDCardConfig.ini");
            idCardLib.SetConfigByFile(configFile);

            // Wait for document with timeout
            boolean documentDetected = false;
            int attempts = 0;
            int maxAttempts = 30;

            while (!documentDetected && attempts < maxAttempts) {
                int docDetectRes = idCardLib.DetectDocument();
                if (docDetectRes == 1) {
                    documentDetected = true;
                    break;
                }
                Thread.sleep(1000);
                attempts++;
            }

            if (!documentDetected) {
                result.put("success", false);
                result.put("error", "No document detected within 30 seconds");
                return result;
            }

            // Process document
            IntByReference cardType = new IntByReference();
            int autoProcessRes = idCardLib.AutoProcessIDCard(cardType);

            if (autoProcessRes == 0) {
                result.put("success", true);
                result.put("passportNumber", getRecogResult(idCardLib, 11));
                result.put("expiry", getRecogResult(idCardLib, 6));
                result.put("issuingCountry", getRecogResult(idCardLib, 7));
                result.put("surname", getRecogResult(idCardLib, 8));
                result.put("englishName", getRecogResult(idCardLib, 3));
                result.put("firstName", getRecogResult(idCardLib, 9));
                result.put("gender", getRecogResult(idCardLib, 4));
                result.put("nationality", getRecogResult(idCardLib, 12));
                result.put("documentNumber", getRecogResult(idCardLib, 13));

                // Save image with timestamp
                String fileName = "/tmp/passport_" + System.currentTimeMillis() + ".jpg";
                int imageRes = idCardLib.SaveImageEx(new WTypes.LPWSTR(fileName), 3);

                if (imageRes == 0) {
                    result.put("imagePath", fileName);
                } else {
                    result.put("imagePath", null);
                }

                // Sound confirmation
                idCardLib.BuzzerAlarm(1000);
            } else {
                result.put("success", false);
                result.put("error", "Failed to process document");
            }

            // Cleanup
            idCardLib.FreeIDCard();

        } catch (Exception e) {
            result.put("success", false);
            result.put("error", "Exception: " + e.getMessage());
        }

        return result;
    }

    private static String getRecogResult(CIDCardRecog idCardLib, int fieldType) {
        WTypes.LPWSTR buffer = new WTypes.LPWSTR("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
        IntByReference bufferLen = new IntByReference(512);

        int result = idCardLib.GetRecogResultEx(1, fieldType, buffer, bufferLen);
        if (result == 0) {
            return String.valueOf(buffer).trim();
        }
        return "";
    }
}