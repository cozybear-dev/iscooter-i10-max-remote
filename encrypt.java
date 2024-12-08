// #PM? - get challenge from scooter
// #+PM>7897C7884748 - challenge from scooter to smartphone
// #+PM<c02fd39e - send proof to scooter from smartphone - NEED THIS
// #+PM>OK OR +PM>NK - OK or not OK reply by scooter to smartpohne
// #+PA<7771126624130 - send challenge to scooter - NEED THIS
// #+PA>8D939C58 - send proof from scooter to smartphone
// #mutual authenticate done

import java.util.Random;
import java.util.Scanner;

public class BleEncryption {
    public static char[] string2Char(String str) {
        char[] cArr = new char[(str.length() / 2)];
        int i = 0;
        while (i < str.length() / 2) {
            int i2 = i + 1;
            cArr[i] = (char) Integer.parseInt(str.substring(i * 2, i2 * 2), 16);
            i = i2;
        }
        return cArr;
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter challenge hex: ");
        String challenge = scanner.nextLine();
        // Create an instance of BleEncryption
        BleEncryption obj = new BleEncryption();
        // Call the encryptionStringOfValue method
        obj.encryptionStringOfValue(challenge);
        obj.getChallenge();
    }

    static String ainv = "9E716E81E1A71CB524884953B2275BBD";
    static String akey = "9883222CD5B257133F3AC675A7132B44";
    static char[] inv = string2Char(ainv);
    static char[] key = string2Char("9883222CD5B257133F3AC675A7132B44");
    static char[] key_tmp = new char[16];
    static char[] s_box = {'c', '|', 'w', '{', 242, 'k', 'o', 197, '0', 1, 'g', '+', 254, 215, 171, 'v', 202, 130, 201, '}', 250, 'Y', 'G', 240, 173, 212, 162, 175, 156, 164, 'r', 192, 183, 253, 147, '&', '6', '?', 247, 204, '4', 165, 229, 241, 'q', 216, '1', 21, 4, 199, '#', 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, '\'', 178, 'u', 9, 131, ',', 26, 27, 'n', 'Z', 160, 'R', ';', 214, 179, ')', 227, '/', 132, 'S', 209, 0, 237, ' ', 252, 177, '[', 'j', 203, 190, '9', 'J', 'L', 'X', 207, 208, 239, 170, 251, 'C', 'M', '3', 133, 'E', 249, 2, 127, 'P', '<', 159, 168, 'Q', 163, '@', 143, 146, 157, '8', 245, 188, 182, 218, '!', 16, 255, 243, 210, 205, 12, 19, 236, '_', 151, 'D', 23, 196, 167, '~', '=', 'd', ']', 25, 's', '`', 129, 'O', 220, '\"', '*', 144, 136, 'F', 238, 184, 20, 222, '^', 11, 219, 224, '2', ':', 10, 'I', 6, '$', '\\', 194, 211, 172, 'b', 145, 149, 228, 'y', 231, 200, '7', 'm', 141, 213, 'N', 169, 'l', 'V', 244, 234, 'e', 'z', 174, 8, 186, 'x', '%', '.', 28, 166, 180, 198, 232, 221, 't', 31, 'K', 189, 139, 138, 'p', '>', 181, 'f', 'H', 3, 246, 14, 'a', '5', 'W', 185, 134, 193, 29, 158, 225, 248, 152, 17, 'i', 217, 142, 148, 155, 30, 135, 233, 206, 'U', '(', 223, 140, 161, 137, 13, 191, 230, 'B', 'h', 'A', 153, '-', 15, 176, 'T', 187, 22};
    char[] bonding_hash = new char[4];
    char[] bonding_nonce = new char[6];

    /* access modifiers changed from: package-private */
    public void sub_bytes(char[] cArr, int i) {
        for (int i2 = 0; i2 < i; i2++) {
            cArr[i2] = s_box[cArr[i2] & 255];
        }
    }

    /* access modifiers changed from: package-private */
    public void bonding_key_generate() {
        char[] cArr = new char[4];
        System.arraycopy(key, 0, key_tmp, 0, 16);
        sub_bytes(key_tmp, 16);
        System.arraycopy(key_tmp, 0, cArr, 0, 4);
        for (char c = 0; c < 12; c = (char) (c + 1)) {
            char[] cArr2 = key_tmp;
            cArr2[c] = cArr2[c + 4];
        }
        System.arraycopy(cArr, 0, key_tmp, 12, 4);
    }

    /* access modifiers changed from: package-private */
    public void bonding_hash_generate() {
        char[] cArr = new char[32];
        System.arraycopy(this.bonding_nonce, 0, cArr, 0, 6);
        System.arraycopy(this.bonding_nonce, 0, cArr, 6, 6);
        System.arraycopy(this.bonding_nonce, 0, cArr, 12, 4);
        System.arraycopy(cArr, 1, cArr, 16, 15);
        cArr[31] = cArr[0];
        bonding_key_generate();
        for (char c = 0; c < 16; c = (char) (c + 1)) {
            char c2 = cArr[c];
            char[] cArr2 = key_tmp;
            char c3 = (char) (c2 ^ cArr2[c]);
            char[] cArr3 = inv;
            cArr[c] = (char) (c3 ^ cArr3[c]);
            int i = c + 16;
            cArr[i] = (char) ((cArr2[c] ^ cArr[i]) ^ cArr3[c]);
        }
        for (char c4 = 1; c4 < 16; c4 = (char) (c4 + 1)) {
            cArr[c4] = (char) (cArr[c4] ^ cArr[c4 - 1]);
            int i2 = c4 + 16;
            cArr[i2] = (char) (cArr[i2] ^ cArr[i2 - 1]);
        }
        for (char c5 = 0; c5 < 4; c5 = (char) (c5 + 1)) {
            this.bonding_hash[c5] = (char) (((((((cArr[c5 + 0] ^ cArr[c5 + 4]) ^ cArr[c5 + 8]) ^ (cArr[c5 + 12] + cArr[c5 + 16])) ^ cArr[c5 + 20]) ^ cArr[c5 + 24]) ^ cArr[c5 + 28]) & 255);
        }
    }

    public String encryptionStringOfValue(String str) {
        // System.out.println("hexString--" + str);
        if (str.length() < 12) {
            return null;
        }
        for (int i = 0; i < 6; i++) {
            int i2 = i * 2;
            String substring = str.substring(i2, i2 + 2);
            // System.out.println("hexString--" + substring);
            this.bonding_nonce[i] = (char) Integer.parseInt(substring, 16);
        }
        bonding_hash_generate();
        String str2 = "";
        for (int i3 = 0; i3 < 4; i3++) {
            // System.out.println("hexString--" + this.bonding_hash[i3] + "---" + Integer.toHexString(this.bonding_hash[i3]));
            String hexString = Integer.toHexString(this.bonding_hash[i3]);
            if (hexString.length() < 2) {
                hexString = "0" + hexString;
            }
            str2 = str2 + hexString;
        }
        System.out.println("--challenge: " + str + "\n--challenge response: " + str2);
        return str2;
    }

    public String getChallenge() {
        Random random = new Random();
        String str2 = "";
        for (int i = 0; i < 6; i++) {
            str2 = str2 + (random.nextInt() & 255);
        }
        System.out.println("--new challenge: " + str2);
        return str2;
    }
}
