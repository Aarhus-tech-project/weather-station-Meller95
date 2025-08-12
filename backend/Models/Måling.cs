using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.EntityFrameworkCore;

namespace VejrstationApi.Models;
//model mappes til db tabel
[Table("readings")]
public class Måling
{
    [Key]
    [Column("id")]
    public int Id { get; set; }

    [Column("ts")]
    public DateTime Tidspunkt { get; set; }

    [Column("temp")]
    [Precision(6, 2)]  // fx  -99.99 .. 999.99 (tilpas efter behov)
    public decimal Temperatur { get; set; }

    [Column("hum")]
    [Precision(5, 2)]  // fx  0.00 .. 100.00
    public decimal Luftfugtighed { get; set; }

    [Column("pres")]
    [Precision(7, 2)]  // fx  0 .. 99999.99 (tilpas efter dine værdier)
    public decimal Tryk { get; set; }
}
