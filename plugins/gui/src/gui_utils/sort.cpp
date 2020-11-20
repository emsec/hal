#include "gui/gui_utils/sort.h"

#include "hal_core/defines.h"

#include <assert.h>

namespace hal
{
    namespace gui_utility
    {
        NumeratedString::NumeratedString(const QString& s)
        {
            bool wasNumerical = false;
            int currentNumber = 0;
            for (const QChar& cc : s)
            {
                if (cc.isDigit())
                {
                    int digit = cc.unicode() - '0';
                    if (wasNumerical)
                        currentNumber = currentNumber * 10 + digit;
                    else
                        currentNumber = digit;
                    wasNumerical = true;
                }
                else
                {
                    if (wasNumerical)
                    {
                        mNumList.append(currentNumber);
                        currentNumber = 0;
                        wasNumerical = false;
                    }
                    mRemainder += cc;
                }
            }
            if (wasNumerical) mNumList.append(currentNumber);
        }

        bool naturalOrderCompare(const QString& a, const QString& b)
        {
            const QChar* a_it = a.constBegin();
            const QChar* b_it = b.constBegin();
            QString a_num, b_num;
            bool number_mode = false;
            while (!a_it->isNull() && !b_it->isNull())
            {
                // read the entire number of each string since we want to be safe
                // against leading zeros: 1 < 0002
                if (a_it->isNumber() || b_it->isNumber())
                {
                    number_mode = true;
                    if (a_it->isNumber())
                    {
                        a_num += *a_it;
                        ++a_it;
                    }
                    if (b_it->isNumber())
                    {
                        b_num += *b_it;
                        ++b_it;
                    }
                }
                else
                {
                    if (number_mode)
                    {
                        int diff = numericStringCompare(a_num, b_num);
                        if (diff != 0) // keep looking if both numbers are equal
                        {
                            return diff < 0;
                        }
                        // reset number cache
                        a_num.clear();
                        b_num.clear();
                        number_mode = false;
                        // no iterator increment here, this block just evaluates
                        // the result of number mode
                    }
                    else if (*a_it != *b_it)
                    {
                        return *a_it < *b_it;
                    }
                    else
                    {
                        ++a_it;
                        ++b_it;
                    }
                }
            }
            // if strings have equal length and end with numbers, compare the
            // numbers
            if (a_it->isNull() && b_it->isNull() && number_mode)
            {
                int diff = numericStringCompare(a_num, b_num);
                if (diff) // keep looking if both numbers are equal
                {
                    return diff < 0;
                }
            }
            return a_it->isNull(); // is a shorter than b?
        }

        bool numeratedOrderCompare(const QString& a, const QString& b)
        {
            NumeratedString ans(a);
            NumeratedString bns(b);

            // check lexical part different
            if (ans.mRemainder < bns.mRemainder) return true;
            if (ans.mRemainder > bns.mRemainder) return false;
            int na = ans.mNumList.size();
            int nb = bns.mNumList.size();
            int n = na < nb ? na : nb;

            // check whether numbers are different
            for (int i=0; i<n; i++)
            {
                if (ans.mNumList.at(i) < bns.mNumList.at(i)) return true;
                if (ans.mNumList.at(i) > bns.mNumList.at(i)) return false;
            }

            // check whether there are additional numbers
            if (na < nb) return true;
            if (nb > na) return false;

            return false;  // all equal
        }

        bool lexicalOrderCompare(const QString& a, const QString& b)
        {
            return a < b;
        }

        int numericStringCompare(QString a_num, QString b_num)
        {
            // this allows us to compare any length of number, regardless of
            // any integer limits

            // zero-pad to same length (we don't care if the strings themselves
            // contain any leading zeros, this still works)
            if (a_num.size() < b_num.size())
            {
                a_num = a_num.rightJustified(b_num.size(), '0');
            }
            else
            {
                b_num = b_num.rightJustified(a_num.size(), '0');
            }
            return a_num.compare(b_num);
        }

        int compare(mSortMechanism mechanism, QString a, QString b)
        {
            switch(mechanism)
            {
            case lexical:
                return lexicalOrderCompare(a, b);
            case natural:
                return naturalOrderCompare(a, b);
            case numerated:
                return numeratedOrderCompare(a, b);
            default:
                assert(false);
            }
            return 0;
        }


    } // namespace gui_utility
}
