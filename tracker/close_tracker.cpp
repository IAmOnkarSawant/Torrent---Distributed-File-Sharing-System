#include <iostream>
#include <string>
#include <cctype> 
using namespace std;

bool isVowel(char c)
{
    c = tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

bool isConsonant(char c)
{
    c = tolower(c);
    return (isalpha(c) && !isVowel(c));
}

int countVal(string s)
{
    
    int ans = 0;

    string word;
    for (int i = 0; i < s.length(); i++)
    {
        if (isalpha(s[i]))
        {
            word += s[i];
        }
        else if (!word.empty())
        {
            bool hasVowel = false;
            bool hasConsonant = false;

            for (char c : word)
            {
                if (isVowel(c))
                {
                    hasVowel = true;
                }
                if (isConsonant(c))
                {
                    hasConsonant = true;
                }
            }

            if (hasVowel && hasConsonant)
            {
                ans++;
            }

            word.clear();
        }
    }

    if (!word.empty())
    {
        bool hasVowel = false;
        bool hasConsonant = false;

        for (char c : word)
        {
            if (isVowel(c))
            {
                hasVowel = true;
            }
            if (isConsonant(c))
            {
                hasConsonant = true;
            }
        }

        if (hasVowel && hasConsonant)
        {
            ans++;
        }
    }
    return ans;
}

int main()
{
    string input;
    cout << "Enter a string: ";
    getline(cin, input);
    cout << countVal(input);

    return 0;
}
