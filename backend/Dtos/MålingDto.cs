namespace VejrstationApi.Dtos;

//dto til at retunere målingsdata
public record MålingDto(DateTime Tidspunkt, decimal Temperatur, decimal Luftfugtighed, decimal Tryk);
