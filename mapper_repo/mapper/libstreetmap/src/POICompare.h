/* 
 * File:   POIpair.h
 * Author: leimiche
 *
 * Created on March 15, 2015, 9:08 PM
 */

#ifndef POICOMPARE_H
#define	POICOMPARE_H

class ComparePOI
{
public:
    bool operator()(pair<unsigned,double>& _lhs, pair<unsigned,double>& _rhs){
        if (_lhs.second>_rhs.second)
            return true;
        else
            return false;
    }    
};

#endif	/* POICOMPARE_H */

