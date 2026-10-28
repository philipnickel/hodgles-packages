// Minimal Caliper consumer: one annotated region, reported by runtime-report.
#include <caliper/cali.h>

int main()
{
   CALI_MARK_BEGIN("caliper_smoke");
   volatile double s = 0.0;
   for (int i = 0; i < 1000000; ++i) { s += 1.0 / (1.0 + i); }
   CALI_MARK_END("caliper_smoke");
   return s > 0.0 ? 0 : 1;
}
